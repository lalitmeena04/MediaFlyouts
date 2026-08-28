#include <QApplication>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "AlbumArtProvider.h"
#include "FlyoutPlacement.h"
#include "NowPlaying.h"
#include "Settings.h"
#include "SingleInstance.h"
#include "ThemeController.h"
#include "TrayController.h"
#include "UpdateChecker.h"
#include "Version.h"

#ifdef Q_OS_WIN
#include <winrt/base.h>

#include "FullscreenDetector.h"
#include "LockKeysController.h"
#include "MediaKeyController.h"
#include "NativeFlyoutSuppressor.h"
#include "StartupController.h"
#include "TaskbarInfo.h"
#include "AudioCapture.h"
#include "AccentWatcher.h"
#include "BatteryStatusController.h"
#include "VolumeController.h"
#include "VolumeMixer.h"
#include "WindowEffects.h"
#include "WinMediaProvider.h"
#endif

int main(int argc, char* argv[]) {
#ifdef Q_OS_WIN
    winrt::init_apartment(winrt::apartment_type::single_threaded);
#endif

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName(QStringLiteral("MediaFlyouts"));
    QApplication::setOrganizationName(QStringLiteral("MediaFlyouts"));
    QApplication::setApplicationVersion(QStringLiteral(MEDIAFLYOUTS_VERSION));

    // a second launch signals the running instance to show its dashboard, then exits
    SingleInstance instance(QStringLiteral("MediaFlyouts.singleInstance"));
    if(!instance.isPrimary())
        return 0;

    // load the icon font in C++ too so the tray glyph can be rendered (QML loads it via FontLoader)
    const int fontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/qt/qml/MediaFlyouts/assets/fonts/FluentSystemIcons-Filled.ttf"));
    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    const QString iconFamily = families.isEmpty() ? QString() : families.first();

    NowPlaying nowPlaying;

    Settings settings;
    FlyoutPlacement placement(&settings);
    ThemeController theme(&settings);
    UpdateChecker updateChecker;

    QQmlApplicationEngine engine;

    // engine takes ownership of the image provider
    auto* artProvider = new AlbumArtProvider();
    engine.addImageProvider(QStringLiteral("albumart"), artProvider);
    nowPlaying.setArtProvider(artProvider);

    TrayController tray(iconFamily, &settings);
    // notify from the tray when an update check finds a newer release
    QObject::connect(&updateChecker, &UpdateChecker::changed, &tray, [&] {
        if(updateChecker.updateAvailable())
            tray.notify(QStringLiteral("MediaFlyouts update available"), updateChecker.status());
    });

#ifdef Q_OS_WIN
    WinMediaProvider provider;
    nowPlaying.setController(&provider);
    QObject::connect(&provider, &WinMediaProvider::trackChanged, &nowPlaying, &NowPlaying::applyTrack);
    QObject::connect(&provider, &WinMediaProvider::artChanged, &nowPlaying, &NowPlaying::applyArt);
    QObject::connect(&provider, &WinMediaProvider::sessionsChanged, &nowPlaying, &NowPlaying::noteSessionsChanged);
    provider.start();

    VolumeController volumeController;
    volumeController.start();
    engine.rootContext()->setContextProperty("volumeController", &volumeController);

    // replace the native Windows volume OSD with our flyout (when enabled). Left alone until
    // onboarding finishes, otherwise neither OSD would show during the wizard.
    NativeFlyoutSuppressor nativeSuppressor;
    if(settings.overrideNativeOsd() && settings.onboardingCompleted())
        nativeSuppressor.start();

    // skip flyouts (and native-OSD suppression) while a fullscreen/exclusive-D3D app is foreground
    FullscreenDetector fullscreen;
    engine.rootContext()->setContextProperty("fullscreen", &fullscreen);
    QObject::connect(&volumeController, &VolumeController::volumeChangedExternally, &nativeSuppressor, [&] {
        if(settings.overrideNativeOsd() && settings.onboardingCompleted()
           && !fullscreen.shouldBlock(settings.disableOnFullscreen()))
            nativeSuppressor.arm();
    });
    // apply the override-native-OSD toggle at runtime
    QObject::connect(&settings, &Settings::changed, &nativeSuppressor, [&] {
        if(settings.overrideNativeOsd() && settings.onboardingCompleted())
            nativeSuppressor.arm();
        else
            nativeSuppressor.restore();
    });

    LockKeysController lockKeys;
    lockKeys.start();
    engine.rootContext()->setContextProperty("lockKeys", &lockKeys);

    // keep the Windows startup (Run) entry in sync with the saved setting
    StartupController startup;
    startup.setEnabled(settings.launchOnStartup());
    QObject::connect(&settings, &Settings::changed, &startup, [&] { startup.setEnabled(settings.launchOnStartup()); });

    // locates the taskbar / tray so the taskbar widget can dock without overlapping the tray
    TaskbarInfo taskbar;
    engine.rootContext()->setContextProperty("taskbar", &taskbar);

    // per-application volume control for the mixer window
    VolumeMixer volumeMixer;
    engine.rootContext()->setContextProperty("volumeMixer", &volumeMixer);

    // WASAPI loopback capture -> FFT bands for the taskbar visualizer
    AudioCapture audioCapture(28);
    engine.rootContext()->setContextProperty("audioCapture", &audioCapture);
    if(settings.taskbarVisualizerEnabled())
        audioCapture.start();
    QObject::connect(&settings, &Settings::changed, &audioCapture, [&] {
        if(settings.taskbarVisualizerEnabled())
            audioCapture.start();
        else
            audioCapture.stop();
    });

    // exposes the themed (dark/light) title-bar helper to QML
    WindowEffects windowEffects;
    engine.rootContext()->setContextProperty("windowEffects", &windowEffects);

    // follow the OS accent color when the theme's accent mode is "match system"
    AccentWatcher accentWatcher;
    QObject::connect(&accentWatcher, &AccentWatcher::accentChanged, &theme, &ThemeController::setSystemAccent);
    accentWatcher.start();
    theme.setSystemAccent(accentWatcher.accent());

    // media keys let us tell a manual skip (media flyout) from an auto-advance (Next Up flyout)
    MediaKeyController mediaKeys;
    mediaKeys.start();
    QObject::connect(&mediaKeys, &MediaKeyController::mediaKeyPressed, &nowPlaying, &NowPlaying::noteMediaKey);
    // volume keys show the flyout even at min/max, where the audio callback doesn't fire
    QObject::connect(&mediaKeys, &MediaKeyController::volumeKeyPressed, &volumeController,
                     &VolumeController::handleVolumeKey);

    // battery level of paired wireless peripherals (headsets, mice, keyboards, ...)
    BatteryStatusController batteryStatus;
    engine.rootContext()->setContextProperty("batteryStatus", &batteryStatus);
#endif

    engine.rootContext()->setContextProperty("nowPlaying", &nowPlaying);
    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("theme", &theme);
    engine.rootContext()->setContextProperty("updateChecker", &updateChecker);
    engine.rootContext()->setContextProperty("placement", &placement);
    engine.rootContext()->setContextProperty("instance", &instance);
    engine.rootContext()->setContextProperty("tray", &tray);
    engine.loadFromModule("MediaFlyouts", "App");

    if(engine.rootObjects().isEmpty())
        return -1;

    tray.show();
    return app.exec();
}
