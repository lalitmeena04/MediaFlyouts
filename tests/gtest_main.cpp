#include <gmock/gmock.h>

#include <QApplication>

#include "TestSupport/IsolatedSettings.h"

int main(int argc, char** argv) {
    // Widget classes such as QFileIconProvider require QApplication.
    QApplication app(argc, argv);
    // Tests must never read or wipe the developer's own settings file.
    mftest::isolateUserSettings();
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
