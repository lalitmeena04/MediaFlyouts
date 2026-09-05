#include "MainControllerClass.h"

#include <QApplication>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>

#include "AlbumArtProvider.h"
#include "Capabilities.h"
#include "FlyoutPlacement.h"
#include "Logging.h"
#include "NowPlaying.h"
#include "QmlLogger.h"
#include "Settings.h"
#include "SingleInstance.h"
#include "ThemeController.h"
#include "TrayController.h"
#include "UpdateChecker.h"
#include "Version.h"

#ifdef Q_OS_WIN
#include "AccentWatcher.h"
#include "AudioCapture.h"
#include "BatteryStatusController.h"
#include "FullscreenDetector.h"
#include "LockKeysController.h"
#include "MediaKeyController.h"
#include "NativeFlyoutSuppressor.h"
#include "PeripheralAlerts.h"
#include "StartupController.h"
#include "TaskbarInfo.h"
#include "VolumeController.h"
#include "VolumeMixer.h"
#include "WindowEffects.h"
#include "WinMediaProvider.h"
#endif

MainControllerClass::MainControllerClass(QApplication& app) : m_app(app) {
    m_app.setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName(QStringLiteral("MediaFlyouts"));
    QApplication::setOrganizationName(QStringLiteral("MediaFlyouts"));
    QApplication::setApplicationVersion(QStringLiteral(MEDIAFLYOUTS_VERSION));

    const QString logFile = mf::initLogging();
    TRC_INFO("==== MediaFlyouts {} starting ====", MEDIAFLYOUTS_VERSION);
    TRC_INFO("log file: {}", logFile.toStdString());
    logScreens();

    // a second launch signals the running instance, then exits (see SingleInstance)
    m_instance = std::make_unique<SingleInstance>(QStringLiteral("MediaFlyouts.singleInstance"));
    if(!m_instance->isPrimary())
        return;
    m_primary = true;

    loadIconFont();
    buildCoreSubsystems();
#ifdef Q_OS_WIN
    buildPlatformSubsystems();
#endif
    registerContextProperties();
    m_engine->loadFromModule("MediaFlyouts", "App");
}

MainControllerClass::~MainControllerClass() = default;

int MainControllerClass::run() {
    if(!m_primary)
        return 0;
    if(m_engine->rootObjects().isEmpty())
        return -1;

    m_tray->show();
    return m_app.exec();
}

void MainControllerClass::logScreens() const {
    for(const QScreen* s : QApplication::screens()) {
        const QRect g = s->geometry();
        const QRect a = s->availableGeometry();
        TRC_DEBUG("screen '{}' geometry=({},{} {}x{}) available=({},{} {}x{}) dpr={} primary={}",
                  s->name().toStdString(), g.x(), g.y(), g.width(), g.height(), a.x(), a.y(), a.width(), a.height(),
                  s->devicePixelRatio(), s == QApplication::primaryScreen());
    }
}

void MainControllerClass::loadIconFont() {
    // load the icon font in C++ too so the tray glyph can be rendered (QML loads it via FontLoader)
    const int fontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/qt/qml/MediaFlyouts/assets/fonts/FluentSystemIcons-Filled.ttf"));
    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    m_iconFamily = families.isEmpty() ? QString() : families.first();
}

void MainControllerClass::buildCoreSubsystems() {
    m_nowPlaying = std::make_unique<NowPlaying>();
    m_settings = std::make_unique<Settings>();
    m_placement = std::make_unique<FlyoutPlacement>(m_settings.get());
    m_theme = std::make_unique<ThemeController>(m_settings.get(), m_nowPlaying.get());
    m_updateChecker = std::make_unique<UpdateChecker>();
    // tracks which platform subsystems came up so the UI can hide/disable dead features
    m_capabilities = std::make_unique<Capabilities>();

    m_engine = std::make_unique<QQmlApplicationEngine>();
    m_qmlLog = std::make_unique<QmlLogger>();

    // engine takes ownership of the image provider
    m_artProvider = new AlbumArtProvider();
    m_engine->addImageProvider(QStringLiteral("albumart"), m_artProvider);
    m_nowPlaying->setArtProvider(m_artProvider);

    m_tray = std::make_unique<TrayController>(m_iconFamily, m_settings.get());
    // notify from the tray when an update check finds a newer release
    QObject::connect(m_updateChecker.get(), &UpdateChecker::changed, m_tray.get(), [this] {
        if(m_updateChecker->updateAvailable())
            m_tray->notify(QStringLiteral("MediaFlyouts update available"), m_updateChecker->status());
    });
}

#ifdef Q_OS_WIN
void MainControllerClass::buildPlatformSubsystems() {
    m_provider = std::make_unique<WinMediaProvider>();
    m_nowPlaying->setController(m_provider.get());
    QObject::connect(m_provider.get(), &WinMediaProvider::trackChanged, m_nowPlaying.get(), &NowPlaying::applyTrack);
    QObject::connect(m_provider.get(), &WinMediaProvider::artChanged, m_nowPlaying.get(), &NowPlaying::applyArt);
    QObject::connect(m_provider.get(), &WinMediaProvider::sessionsChanged, m_nowPlaying.get(),
                     &NowPlaying::noteSessionsChanged);
    m_provider->start();
    m_capabilities->setMediaSessionAvailable(m_provider->available());
    QObject::connect(m_provider.get(), &WinMediaProvider::availabilityChanged, m_capabilities.get(),
                     [this] { m_capabilities->setMediaSessionAvailable(m_provider->available()); });

    m_volumeController = std::make_unique<VolumeController>();
    m_volumeController->start();
    m_capabilities->setAudioAvailable(m_volumeController->available());
    QObject::connect(m_volumeController.get(), &VolumeController::availabilityChanged, m_capabilities.get(),
                     [this] { m_capabilities->setAudioAvailable(m_volumeController->available()); });

    // replace the native Windows volume OSD with our flyout (when enabled). Left alone until
    // onboarding finishes, otherwise neither OSD would show during the wizard.
    m_nativeSuppressor = std::make_unique<NativeFlyoutSuppressor>();
    if(m_settings->overrideNativeOsd() && m_settings->onboardingCompleted())
        m_nativeSuppressor->start();

    // skip flyouts (and native-OSD suppression) while a fullscreen/exclusive-D3D app is foreground
    m_fullscreen = std::make_unique<FullscreenDetector>();
    QObject::connect(m_volumeController.get(), &VolumeController::volumeChangedExternally, m_nativeSuppressor.get(),
                     [this] {
                         if(m_settings->overrideNativeOsd() && m_settings->onboardingCompleted()
                            && !m_fullscreen->shouldBlock(m_settings->disableOnFullscreen(),
                                                          m_settings->volumeFlyoutMonitor()))
                             m_nativeSuppressor->arm();
                     });
    // apply the override-native-OSD toggle at runtime
    QObject::connect(m_settings.get(), &Settings::changed, m_nativeSuppressor.get(), [this] {
        if(m_settings->overrideNativeOsd() && m_settings->onboardingCompleted())
            m_nativeSuppressor->arm();
        else
            m_nativeSuppressor->restore();
    });

    m_lockKeys = std::make_unique<LockKeysController>();
    m_lockKeys->start();
    m_capabilities->setLockKeysAvailable(m_lockKeys->available());
    QObject::connect(m_lockKeys.get(), &LockKeysController::availabilityChanged, m_capabilities.get(),
                     [this] { m_capabilities->setLockKeysAvailable(m_lockKeys->available()); });

    // keep the Windows startup (Run) entry in sync with the saved setting
    m_startup = std::make_unique<StartupController>();
    m_startup->setEnabled(m_settings->launchOnStartup());
    QObject::connect(m_settings.get(), &Settings::changed, m_startup.get(),
                     [this] { m_startup->setEnabled(m_settings->launchOnStartup()); });

    // locates the taskbar / tray so the taskbar widget can dock without overlapping the tray
    m_taskbar = std::make_unique<TaskbarInfo>();

    // per-application volume control for the mixer window
    m_volumeMixer = std::make_unique<VolumeMixer>();

    // WASAPI loopback capture -> FFT bands for the taskbar visualizer. Capture is started and
    // stopped from QML based on whether the visualizer is actually on screen (see TaskbarWidget).
    m_audioCapture = std::make_unique<AudioCapture>(28);

    // exposes the themed (dark/light) title-bar helper to QML
    m_windowEffects = std::make_unique<WindowEffects>();

    // follow the OS accent color when the theme's accent mode is "match system"
    m_accentWatcher = std::make_unique<AccentWatcher>();
    QObject::connect(m_accentWatcher.get(), &AccentWatcher::accentChanged, m_theme.get(),
                     &ThemeController::setSystemAccent);
    m_accentWatcher->start();
    m_theme->setSystemAccent(m_accentWatcher->accent());

    // media keys let us tell a manual skip (media flyout) from an auto-advance (Next Up flyout)
    m_mediaKeys = std::make_unique<MediaKeyController>();
    m_mediaKeys->start();
    QObject::connect(m_mediaKeys.get(), &MediaKeyController::mediaKeyPressed, m_nowPlaying.get(),
                     &NowPlaying::noteMediaKey);
    // volume keys show the flyout even at min/max, where the audio callback doesn't fire
    QObject::connect(m_mediaKeys.get(), &MediaKeyController::volumeKeyPressed, m_volumeController.get(),
                     &VolumeController::handleVolumeKey);
    m_capabilities->setMediaKeysAvailable(m_mediaKeys->available());
    QObject::connect(m_mediaKeys.get(), &MediaKeyController::availabilityChanged, m_capabilities.get(),
                     [this] { m_capabilities->setMediaKeysAvailable(m_mediaKeys->available()); });

    // battery level of paired wireless peripherals (headsets, mice, keyboards, ...)
    m_batteryStatus = std::make_unique<BatteryStatusController>();

    m_peripheralAlerts = std::make_unique<PeripheralAlerts>(m_batteryStatus.get());

    // a low battery can happen at any moment, so it also gets a tray notification
    QObject::connect(m_peripheralAlerts.get(), &PeripheralAlerts::lowBattery, m_tray.get(),
                     [this](const QString& device, int percent, bool veryLow) {
                         if(!m_settings->peripheralLowBatteryToast() || !m_settings->onboardingCompleted())
                             return;
                         m_tray->notify(veryLow ? QStringLiteral("Peripheral battery very low")
                                                : QStringLiteral("Peripheral battery low"),
                                        QStringLiteral("%1 is at %2%").arg(device).arg(percent));
                     });
}
#endif

void MainControllerClass::registerContextProperties() {
    QQmlContext* ctx = m_engine->rootContext();
    ctx->setContextProperty("log", m_qmlLog.get());
    ctx->setContextProperty("nowPlaying", m_nowPlaying.get());
    ctx->setContextProperty("settings", m_settings.get());
    ctx->setContextProperty("capabilities", m_capabilities.get());
    ctx->setContextProperty("theme", m_theme.get());
    ctx->setContextProperty("updateChecker", m_updateChecker.get());
    ctx->setContextProperty("placement", m_placement.get());
    ctx->setContextProperty("instance", m_instance.get());
    ctx->setContextProperty("tray", m_tray.get());
#ifdef Q_OS_WIN
    ctx->setContextProperty("volumeController", m_volumeController.get());
    ctx->setContextProperty("fullscreen", m_fullscreen.get());
    ctx->setContextProperty("lockKeys", m_lockKeys.get());
    ctx->setContextProperty("taskbar", m_taskbar.get());
    ctx->setContextProperty("volumeMixer", m_volumeMixer.get());
    ctx->setContextProperty("audioCapture", m_audioCapture.get());
    ctx->setContextProperty("windowEffects", m_windowEffects.get());
    ctx->setContextProperty("batteryStatus", m_batteryStatus.get());
    ctx->setContextProperty("peripheralAlerts", m_peripheralAlerts.get());
#endif
}
