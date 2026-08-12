#include <gmock/gmock.h>

#include <QCoreApplication>

// A QCoreApplication is required for the Qt objects under test (QTimer, QSettings).
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
