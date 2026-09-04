#include <QApplication>

#ifdef Q_OS_WIN
#include <winrt/base.h>
#endif

#include "MainControllerClass.h"

int main(int argc, char* argv[]) {
#ifdef Q_OS_WIN
    winrt::init_apartment(winrt::apartment_type::single_threaded);
#endif

    QApplication app(argc, argv);
    MainControllerClass controller(app);
    return controller.run();
}
