#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

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

#ifdef Q_OS_WIN
    WinMediaProvider provider;
    nowPlaying.setController(&provider);
    QObject::connect(&provider, &WinMediaProvider::trackChanged,
                     &nowPlaying, &NowPlaying::applyTrack);
    provider.start();
#endif

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("nowPlaying", &nowPlaying);
    engine.loadFromModule("MediaFlyouts", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
