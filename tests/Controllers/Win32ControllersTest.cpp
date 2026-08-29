#include <gtest/gtest.h>

#include <QVariant>
#include <QVariantMap>
#include <QCoreApplication>

#include <windows.h>

#include <objbase.h>

#include "AudioCapture.h"
#include "BluetoothConnectionWatcher.h"
#include "HidChangeWatcher.h"
#include "TaskbarInfo.h"
#include "VolumeMixer.h"
#include "WindowEffects.h"

// These Windows-only controllers wrap COM / DWM APIs; without a device or window
// we can still verify their construction and out-of-range operations are safe.

TEST(Win32ControllersTest, AudioCaptureStartsSilentAndInactive) {
    AudioCapture capture(24);
    EXPECT_FALSE(capture.active());
    EXPECT_EQ(capture.bands().size(), 24);
    EXPECT_DOUBLE_EQ(capture.level(), 0.0);
    for(const QVariant& band : capture.bands())
        EXPECT_DOUBLE_EQ(band.toDouble(), 0.0);
}

TEST(Win32ControllersTest, VolumeMixerDefaultsEmptyAndIgnoresBadIndices) {
    VolumeMixer mixer;
    EXPECT_TRUE(mixer.sessions().isEmpty());
    mixer.setSessionVolume(0, 50);
    mixer.setSessionVolume(-1, 10);
    mixer.setSessionMute(3, true);
    EXPECT_TRUE(mixer.sessions().isEmpty());
}

TEST(Win32ControllersTest, WindowEffectsToleratesNullWindow) {
    WindowEffects effects;
    effects.setDarkFrame(nullptr, true);
    effects.setDarkFrame(nullptr, false);
    SUCCEED();
}

TEST(Win32ControllersTest, DeviceWatchersCanStartRepeatedly) {
    BluetoothConnectionWatcher bluetooth;
    HidChangeWatcher hid;

    bluetooth.start();
    bluetooth.start();
    hid.start();
    hid.start();
    QCoreApplication::processEvents();
    SUCCEED();
}

TEST(Win32ControllersTest, TaskbarWidgetPositionsStayOnTheSelectedScreen) {
    TaskbarInfo taskbar;
    const QRect screen = taskbar.monitorGeometry(0);
    if(screen.isEmpty())
        GTEST_SKIP() << "No QScreen is available in the guiless test host.";

    constexpr int width = 220;
    constexpr int height = 40;
    for(int side : {0, 1, 2}) {
        const QPoint position = taskbar.widgetPosition(0, side, 0, width, height);
        EXPECT_GE(position.x(), screen.left());
        EXPECT_LE(position.x() + width, screen.right() + 1);
        EXPECT_GE(position.y(), screen.top());
        EXPECT_LE(position.y() + height, screen.bottom() + 1);
    }
}

// With COM initialised we can exercise the real enumeration / capture code paths.
// These degrade gracefully whether or not the CI runner exposes an audio device.
class Win32ComTest : public ::testing::Test {
    protected:
        void SetUp() override {
            CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        }
        void TearDown() override {
            CoUninitialize();
        }
};

TEST_F(Win32ComTest, VolumeMixerRefreshProducesValidSessions) {
    VolumeMixer mixer;
    mixer.refresh();
    for(const QVariant& entry : mixer.sessions()) {
        const QVariantMap m = entry.toMap();
        EXPECT_GE(m.value(QStringLiteral("volume")).toInt(), 0);
        EXPECT_LE(m.value(QStringLiteral("volume")).toInt(), 100);
        EXPECT_FALSE(m.value(QStringLiteral("name")).toString().isEmpty());
    }
}

TEST_F(Win32ComTest, AudioCaptureStartStopIsSafe) {
    AudioCapture capture(16);
    capture.start();
    capture.stop();
    EXPECT_FALSE(capture.active());
    EXPECT_EQ(capture.bands().size(), 16);
}
