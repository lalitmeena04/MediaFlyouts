#include "MainControllerClass.h"

#include <QApplication>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSysInfo>

#include "AlbumArtProvider.h"
#include "AppIcon.h"
#include "AppRestarter.h"
#include "AppTranslator.h"
#include "Capabilities.h"
#include "Donation.h"
#include "DonationController.h"
#include "EntitlementController.h"
#include "EntitlementStore.h"
#include "ErrorReporter.h"
#include "FlyoutPlacement.h"
#include "InstallSource.h"
#include "Logging.h"
#include "MonitorMapper.h"
#include "NowPlaying.h"
#include "ProBuildConfig.h"
#include "ProGate.h"
#include "QmlLogger.h"
#include "QrCode.h"
#include "Settings.h"
#include "SingleInstance.h"
#include "SwipeDetector.h"
#include "ThemeController.h"
#include "TrayController.h"
#include "UpdateChecker.h"
#include "Version.h"
#include "WindowScaleRefresher.h"

#ifdef Q_OS_WIN
#include "AccentWatcher.h"
#include "AudioCapture.h"
#include "BatteryStatusController.h"
#include "FullscreenDetector.h"
#include "LockKeysController.h"
#include "MediaKeyController.h"
#include "NativeFlyoutSuppressor.h"
#include "PeripheralAlerts.h"
#include "PowerStatusController.h"
#include "StartupController.h"
#include "StoreLicensing.h"
#include "TaskbarBackdrop.h"
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
    TRC_INFO("os: {} ({}), qt {}", QSysInfo::prettyProductName().toStdString(), QSysInfo::kernelVersion().toStdString(),
             qVersion());
    // Every QML Window inherits this icon for its frame and taskbar button.
    QApplication::setWindowIcon(mf::loadAppIcon());

    // A restarted instance must not hand over to the instance it replaces.
    AppRestarter::waitForPredecessor(QCoreApplication::arguments());
    m_instance = std::make_unique<SingleInstance>(QStringLiteral("MediaFlyouts.singleInstance"));
    if(!m_instance->isPrimary())
        return;
    m_primary = true;

    loadIconFont();
    buildCoreSubsystems();
#ifdef Q_OS_WIN
    buildPlatformSubsystems();
#endif
    // After the platform gateway exists, so the first Store refresh runs at startup.
    m_entitlement->start();
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
    const int code = m_app.exec();
    TRC_INFO("==== MediaFlyouts exiting (code {}) ====", code);
    mf::flushLogging();
    return code;
}

void MainControllerClass::loadIconFont() {
    const int fontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/qt/qml/MediaFlyouts/assets/fonts/FluentSystemIcons-Filled.ttf"));
    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    m_iconFamily = families.isEmpty() ? QString() : families.first();
}

void MainControllerClass::buildCoreSubsystems() {
    m_nowPlaying = std::make_unique<NowPlaying>();
    m_settings = std::make_unique<Settings>();
    // Translations go in before any UI text is built.
    m_translator = std::make_unique<AppTranslator>();
    m_translator->apply(m_settings->language());
    QObject::connect(m_settings.get(), &Settings::changed, m_translator.get(),
                     [this] { m_translator->apply(m_settings->language()); });
    m_monitors = std::make_unique<MonitorMapper>();
    m_monitors->count();
    m_placement = std::make_unique<FlyoutPlacement>(m_settings.get(), m_monitors.get());
    // First listener on monitors.changed. Sizes are right before QML re-places windows.
    m_scaleRefresher = std::make_unique<WindowScaleRefresher>(m_monitors.get());
    m_swipe = std::make_unique<SwipeDetector>();
    m_theme = std::make_unique<ThemeController>(m_settings.get(), m_nowPlaying.get());
    m_updateChecker = std::make_unique<UpdateChecker>();
    m_capabilities = std::make_unique<Capabilities>();
    m_errorReporter = std::make_unique<ErrorReporter>(m_settings.get(), m_capabilities.get());
    m_restarter = std::make_unique<AppRestarter>(m_errorReporter.get());
    m_entitlement = std::make_unique<EntitlementController>(mf::detectInstallKind(), mf::proBuildConfig(),
                                                            std::make_unique<mf::EntitlementStore>());
    m_proGate = std::make_unique<ProGate>(m_settings.get(), m_entitlement.get());
    m_donation = std::make_unique<DonationController>(mf::donationConfig(), m_entitlement->installKind(),
                                                      !mf::proBuildConfig().tipAddOnStoreIds.isEmpty());

    m_engine = std::make_unique<QQmlApplicationEngine>();
    m_translator->setEngine(m_engine.get());
    m_qmlLog = std::make_unique<QmlLogger>();

    m_artProvider = new AlbumArtProvider();
    m_engine->addImageProvider(QStringLiteral("albumart"), m_artProvider);
    m_engine->addImageProvider(QStringLiteral("qr"), new QrImageProvider());
    m_nowPlaying->setArtProvider(m_artProvider);

    m_tray = std::make_unique<TrayController>(m_iconFamily, m_settings.get());
    m_tray->trackCapabilities(m_capabilities.get());
    QObject::connect(m_translator.get(), &AppTranslator::languageChanged, m_tray.get(), &TrayController::retranslate);
    // The shell does not always re-query the icon after a DPI change.
    QObject::connect(m_monitors.get(), &MonitorMapper::changed, m_tray.get(), &TrayController::refreshIcon);
    QObject::connect(m_updateChecker.get(), &UpdateChecker::installStarted, &m_app, &QApplication::quit);
}

#ifdef Q_OS_WIN
void MainControllerClass::buildPlatformSubsystems() {
    m_storeLicensing = std::make_unique<StoreLicensing>(mf::proBuildConfig().proAddOnStoreId,
                                                        mf::proBuildConfig().tipAddOnStoreIds);
    m_entitlement->setGateway(m_storeLicensing.get());

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

    m_nativeSuppressor = std::make_unique<NativeFlyoutSuppressor>();
    if(m_settings->overrideNativeOsd() && m_settings->onboardingCompleted())
        m_nativeSuppressor->start();

    m_fullscreen = std::make_unique<FullscreenDetector>(m_monitors.get());
    QObject::connect(m_volumeController.get(), &VolumeController::volumeChangedExternally, m_nativeSuppressor.get(),
                     [this] {
                         if(m_settings->overrideNativeOsd() && m_settings->onboardingCompleted()
                            && !m_fullscreen->shouldBlock(m_settings->disableOnFullscreen(),
                                                          m_settings->volumeFlyoutMonitor()))
                             m_nativeSuppressor->arm();
                     });
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

    m_startup = std::make_unique<StartupController>();
    m_startup->setEnabled(m_settings->launchOnStartup());
    QObject::connect(m_settings.get(), &Settings::changed, m_startup.get(),
                     [this] { m_startup->setEnabled(m_settings->launchOnStartup()); });

    m_taskbar = std::make_unique<TaskbarInfo>(m_monitors.get());
    m_taskbarBackdrop = std::make_unique<TaskbarBackdrop>();

    m_volumeMixer = std::make_unique<VolumeMixer>();

    m_audioCapture = std::make_unique<AudioCapture>(28);

    m_windowEffects = std::make_unique<WindowEffects>();

    m_accentWatcher = std::make_unique<AccentWatcher>();
    QObject::connect(m_accentWatcher.get(), &AccentWatcher::accentChanged, m_theme.get(),
                     &ThemeController::setSystemAccent);
    m_accentWatcher->start();
    m_theme->setSystemAccent(m_accentWatcher->accent());

    m_mediaKeys = std::make_unique<MediaKeyController>();
    m_mediaKeys->start();
    QObject::connect(m_mediaKeys.get(), &MediaKeyController::mediaKeyPressed, m_nowPlaying.get(),
                     &NowPlaying::noteMediaKey);
    QObject::connect(m_mediaKeys.get(), &MediaKeyController::volumeKeyPressed, m_volumeController.get(),
                     &VolumeController::handleVolumeKey);
    m_capabilities->setMediaKeysAvailable(m_mediaKeys->available());
    QObject::connect(m_mediaKeys.get(), &MediaKeyController::availabilityChanged, m_capabilities.get(),
                     [this] { m_capabilities->setMediaKeysAvailable(m_mediaKeys->available()); });

    m_batteryStatus = std::make_unique<BatteryStatusController>();

    m_peripheralAlerts = std::make_unique<PeripheralAlerts>(m_batteryStatus.get());

    QObject::connect(m_peripheralAlerts.get(), &PeripheralAlerts::lowBattery, m_tray.get(),
                     [this](const QString& device, int percent, bool veryLow) {
                         if(!m_proGate->peripheralLowBatteryToast() || !m_settings->onboardingCompleted())
                             return;
                         if(!m_settings->peripheralSelected(device))
                             return;
                         m_tray->notify(veryLow ? TrayController::tr("Peripheral battery very low")
                                                : TrayController::tr("Peripheral battery low"),
                                        TrayController::tr("%1 is at %2%")
                                            .arg(m_settings->peripheralDisplayName(device))
                                            .arg(percent));
                     });

    m_powerStatus = std::make_unique<PowerStatusController>();
    m_powerStatus->start();
    m_capabilities->setChargerAvailable(m_powerStatus->available());
    QObject::connect(m_powerStatus.get(), &PowerStatusController::availabilityChanged, m_capabilities.get(),
                     [this] { m_capabilities->setChargerAvailable(m_powerStatus->available()); });
}
#endif

void MainControllerClass::registerContextProperties() {
    QQmlContext* ctx = m_engine->rootContext();
    ctx->setContextProperty("log", m_qmlLog.get());
    ctx->setContextProperty("nowPlaying", m_nowPlaying.get());
    ctx->setContextProperty("settings", m_settings.get());
    ctx->setContextProperty("capabilities", m_capabilities.get());
    ctx->setContextProperty("entitlement", m_entitlement.get());
    ctx->setContextProperty("pro", m_proGate.get());
    ctx->setContextProperty("donate", m_donation.get());
    ctx->setContextProperty("theme", m_theme.get());
    ctx->setContextProperty("updateChecker", m_updateChecker.get());
    ctx->setContextProperty("errorReporter", m_errorReporter.get());
    ctx->setContextProperty("restarter", m_restarter.get());
    ctx->setContextProperty("placement", m_placement.get());
    ctx->setContextProperty("swipe", m_swipe.get());
    ctx->setContextProperty("monitors", m_monitors.get());
    ctx->setContextProperty("instance", m_instance.get());
    ctx->setContextProperty("tray", m_tray.get());
    ctx->setContextProperty("i18n", m_translator.get());
#ifdef Q_OS_WIN
    ctx->setContextProperty("volumeController", m_volumeController.get());
    ctx->setContextProperty("fullscreen", m_fullscreen.get());
    ctx->setContextProperty("lockKeys", m_lockKeys.get());
    ctx->setContextProperty("taskbar", m_taskbar.get());
    ctx->setContextProperty("taskbarBackdrop", m_taskbarBackdrop.get());
    ctx->setContextProperty("volumeMixer", m_volumeMixer.get());
    ctx->setContextProperty("audioCapture", m_audioCapture.get());
    ctx->setContextProperty("windowEffects", m_windowEffects.get());
    ctx->setContextProperty("batteryStatus", m_batteryStatus.get());
    ctx->setContextProperty("peripheralAlerts", m_peripheralAlerts.get());
    ctx->setContextProperty("powerStatus", m_powerStatus.get());
#endif
}
