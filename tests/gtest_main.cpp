#include <gmock/gmock.h>

#include <QGuiApplication>

int main(int argc, char** argv) {
    // QGuiApplication gives tests real QScreen geometry. Set QT_QPA_PLATFORM=offscreen on hosts without a desktop.
    QGuiApplication app(argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
