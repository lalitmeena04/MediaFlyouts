#include <gtest/gtest.h>

#include <QVariant>
#include <QVariantMap>
#include <QCoreApplication>

#include <windows.h>

#include <objbase.h>

#include "AudioCapture.h"
#include "BatteryProviders/WiredPeripheralProvider.h"
#include "BluetoothConnectionWatcher.h"
#include "HidChangeWatcher.h"
#include "MonitorMapper.h"
#include "PowerStatusController.h"
#include "TaskbarInfo.h"
#include "VolumeMixer.h"
#include "WindowEffects.h"
#include "WiredPeripheralRules.h"

// These Windows controllers wrap COM and DWM APIs.
// Tests use no real device or window.

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
    effects.pinTopmost(nullptr);
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

// Runs against whatever is plugged into the test machine.
// Every entry must look like a wired peripheral, never a disk.
TEST(Win32ControllersTest, WiredProviderReportsUsableDevicesOnly) {
    WiredPeripheralProvider provider;
    const QVector<BatteryDeviceInfo> devices = provider.poll();

    for(const BatteryDeviceInfo& device : devices) {
        EXPECT_EQ(device.connectionKind, QStringLiteral("usb"));
        EXPECT_EQ(device.percent, -1);
        EXPECT_FALSE(device.name.trimmed().isEmpty());
        EXPECT_FALSE(mf::isGenericDeviceName(device.name));
    }
}

// The first reading only seeds the baseline, so no charger event fires at startup.
TEST(Win32ControllersTest, PowerStatusStartRaisesNoChargerEvent) {
    PowerStatusController power;
    int events = 0;
    QObject::connect(&power, &PowerStatusController::chargerChanged, &power, [&events] { ++events; });

    power.start();
    power.refresh();
    QCoreApplication::processEvents();

    EXPECT_EQ(events, 0);
    EXPECT_GE(power.percent(), -1);
    EXPECT_LE(power.percent(), 100);
}

TEST(Win32ControllersTest, TaskbarWidgetPositionsStayOnTheSelectedScreen) {
    MonitorMapper monitors;
    TaskbarInfo taskbar(&monitors);
    taskbar.pinTopmost(nullptr);
    taskbar.pinAboveTaskbar(nullptr);
    const QRect screen = monitors.geometry(0);
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

// COM lets tests use the real list and capture paths.
// Tests pass with or without an audio device.
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

// A right-aligned widget must stop left of the tray drawn on ITS monitor.
// Before the fix, secondary monitors reused the primary tray width.
TEST_F(Win32ComTest, RightAlignedWidgetClearsEachMonitorsOwnTray) {
    MonitorMapper monitors;
    TaskbarInfo taskbar(&monitors);
    if(monitors.count() == 0)
        GTEST_SKIP() << "No QScreen is available in the guiless test host.";

    constexpr int width = 220;
    constexpr int height = 40;
    constexpr int right = static_cast<int>(mf::TaskbarSide::Right);
    for(int i = 0; i < monitors.count(); ++i) {
        const QRect screen = monitors.geometry(i);
        const QPoint position = taskbar.widgetPosition(i, right, 0, width, height);
        // Never off the right edge, never pushed past the whole screen.
        EXPECT_LT(position.x() + width, screen.right() + 1) << "monitor " << i;
        EXPECT_GE(position.x(), screen.left()) << "monitor " << i;
        // Clock and notification area are always narrower than half the taskbar.
        EXPECT_GT(position.x() + width, screen.left() + screen.width() / 2) << "monitor " << i;
    }
}

// Repeated polls must hit the cache. The automation walk is slow.
TEST_F(Win32ComTest, WidgetPositionIsStableAcrossRapidPolls) {
    MonitorMapper monitors;
    TaskbarInfo taskbar(&monitors);
    if(monitors.count() == 0)
        GTEST_SKIP() << "No QScreen is available in the guiless test host.";

    const int last = monitors.count() - 1;
    const QPoint first = taskbar.widgetPosition(last, 2, 0, 220, 40);
    for(int i = 0; i < 20; ++i)
        EXPECT_EQ(taskbar.widgetPosition(last, 2, 0, 220, 40), first);
}
