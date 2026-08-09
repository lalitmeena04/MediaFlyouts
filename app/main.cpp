#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "albumartprovider.h"
#include "nowplaying.h"

#ifdef Q_OS_WIN
#include <winrt/base.h>

#include "winmediaprovider.h"
#endif

int main(int argc, char* argv[])
{
#ifdef Q_OS_WIN
    winrt::init_apartment(winrt::apartment_type::single_threaded);
#endif

    QGuiApplication app(argc, argv);

    NowPlaying nowPlaying;

    QQmlApplicationEngine engine;

    const int iconFontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/qt/qml/MediaFlyouts/assets/fonts/FluentSystemIcons-Filled.ttf"));
    const QStringList iconFamilies = QFontDatabase::applicationFontFamilies(iconFontId);
    engine.rootContext()->setContextProperty(
        "iconFontFamily", iconFamilies.isEmpty() ? QString() : iconFamilies.first());

    // engine takes ownership of the image provider
    auto* artProvider = new AlbumArtProvider();
    engine.addImageProvider(QStringLiteral("albumart"), artProvider);
    nowPlaying.setArtProvider(artProvider);

#ifdef Q_OS_WIN
    WinMediaProvider provider;
    nowPlaying.setController(&provider);
    QObject::connect(&provider, &WinMediaProvider::trackChanged,
                     &nowPlaying, &NowPlaying::applyTrack);
    QObject::connect(&provider, &WinMediaProvider::artChanged,
                     &nowPlaying, &NowPlaying::applyArt);
    provider.start();
#endif

    engine.rootContext()->setContextProperty("nowPlaying", &nowPlaying);
    engine.loadFromModule("MediaFlyouts", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
