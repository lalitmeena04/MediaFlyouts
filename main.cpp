#include <QApplication>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "AlbumArtProvider.h"
#include "NowPlaying.h"
#include "Settings.h"
#include "TrayController.h"

#ifdef Q_OS_WIN
#include <winrt/base.h>

#include "FullscreenDetector.h"
#include "LockKeysController.h"
#include "MediaKeyController.h"
#include "NativeFlyoutSuppressor.h"
#include "VolumeController.h"
#include "WinMediaProvider.h"
#endif

int main(int argc, char* argv[]) {
#ifdef Q_OS_WIN
    winrt::init_apartment(winrt::apartment_type::single_threaded);
#endif

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // load the icon font in C++ too so the tray glyph can be rendered (QML loads it via FontLoader)
    const int fontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/qt/qml/MediaFlyouts/assets/fonts/FluentSystemIcons-Filled.ttf"));
    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    const QString iconFamily = families.isEmpty() ? QString() : families.first();

    NowPlaying nowPlaying;

    Settings settings;

    QQmlApplicationEngine engine;

    // engine takes ownership of the image provider
    auto* artProvider = new AlbumArtProvider();
    engine.addImageProvider(QStringLiteral("albumart"), artProvider);
    nowPlaying.setArtProvider(artProvider);

    TrayController tray(iconFamily);

#ifdef Q_OS_WIN
    WinMediaProvider provider;
    nowPlaying.setController(&provider);
    QObject::connect(&provider, &WinMediaProvider::trackChanged, &nowPlaying, &NowPlaying::applyTrack);
    QObject::connect(&provider, &WinMediaProvider::artChanged, &nowPlaying, &NowPlaying::applyArt);
    provider.start();

    VolumeController volumeController;
    volumeController.start();
    engine.rootContext()->setContextProperty("volumeController", &volumeController);

    // replace the native Windows volume OSD with our flyout (when enabled)
    NativeFlyoutSuppressor nativeSuppressor;
    if(settings.overrideNativeOsd())
        nativeSuppressor.start();

    // skip flyouts (and native-OSD suppression) while a fullscreen/exclusive-D3D app is foreground
    FullscreenDetector fullscreen;
    engine.rootContext()->setContextProperty("fullscreen", &fullscreen);
    QObject::connect(&volumeController, &VolumeController::volumeChangedExternally, &nativeSuppressor, [&] {
        if(settings.overrideNativeOsd() && !fullscreen.shouldBlock(settings.disableOnFullscreen()))
            nativeSuppressor.arm();
    });
    // apply the override-native-OSD toggle at runtime
    QObject::connect(&settings, &Settings::changed, &nativeSuppressor, [&] {
        if(settings.overrideNativeOsd())
            nativeSuppressor.arm();
        else
            nativeSuppressor.restore();
    });

    LockKeysController lockKeys;
    lockKeys.start();
    engine.rootContext()->setContextProperty("lockKeys", &lockKeys);

    // media keys let us tell a manual skip (media flyout) from an auto-advance (Next Up flyout)
    MediaKeyController mediaKeys;
    mediaKeys.start();
    QObject::connect(&mediaKeys, &MediaKeyController::mediaKeyPressed, &nowPlaying, &NowPlaying::noteMediaKey);
    // volume keys show the flyout even at min/max, where the audio callback doesn't fire
    QObject::connect(&mediaKeys, &MediaKeyController::volumeKeyPressed, &volumeController,
                     &VolumeController::handleVolumeKey);
#endif

    engine.rootContext()->setContextProperty("nowPlaying", &nowPlaying);
    engine.rootContext()->setContextProperty("settings", &settings);
    engine.rootContext()->setContextProperty("tray", &tray);
    engine.loadFromModule("MediaFlyouts", "App");

    if(engine.rootObjects().isEmpty())
        return -1;

    tray.show();
    return app.exec();
}
