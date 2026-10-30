#include <gmock/gmock.h>

#include <QGuiApplication>

#include "TestSupport/IsolatedSettings.h"

int main(int argc, char** argv) {
    // QGuiApplication gives tests real QScreen geometry. Set QT_QPA_PLATFORM=offscreen on hosts without a desktop.
    QGuiApplication app(argc, argv);
    // Tests must never read or wipe the developer's own settings file.
    mftest::isolateUserSettings();
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
