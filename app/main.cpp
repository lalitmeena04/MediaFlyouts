#include <QApplication>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "albumartprovider.h"
#include "nowplaying.h"
#include "traycontroller.h"

#ifdef Q_OS_WIN
#include <winrt/base.h>

#include "winmediaprovider.h"
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
#endif

    engine.rootContext()->setContextProperty("nowPlaying", &nowPlaying);
    engine.rootContext()->setContextProperty("tray", &tray);
    engine.loadFromModule("MediaFlyouts", "App");

    if(engine.rootObjects().isEmpty())
        return -1;

    tray.show();
    return app.exec();
}
