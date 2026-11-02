#include <QApplication>
#include <QSettings>

#ifdef Q_OS_WIN
#include <winrt/base.h>
#endif

#include "MainControllerClass.h"

int main(int argc, char* argv[]) {
#ifdef Q_OS_WIN
    winrt::init_apartment(winrt::apartment_type::single_threaded);
#endif

    QApplication app(argc, argv);
#ifndef QT_NO_DEBUG
    // Lets tools (screenshot capture, UI automation) run the app on a scratch settings folder.
    const QString settingsDir = qEnvironmentVariable("MEDIAFLYOUTS_SETTINGS_DIR");
    if(!settingsDir.isEmpty())
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir);
#endif
    MainControllerClass controller(app);
    return controller.run();
}
