#include <gtest/gtest.h>

#include <QVariant>
#include <QVariantMap>
#include <QCoreApplication>
#include <QSignalSpy>

#include <windows.h>

#include <objbase.h>

#include "AudioCapture.h"
#include "BatteryProviders/WiredPeripheralProvider.h"
#include "BluetoothConnectionWatcher.h"
#include "HidChangeWatcher.h"
#include "MonitorMapper.h"
#include "PowerStatusController.h"
#include "StoreLicensing.h"
#include "TaskbarBackdrop.h"
#include "TaskbarInfo.h"
#include "VolumeMixer.h"
#include "WindowEffects.h"
#include "WiredPeripheralRules.h"

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

TEST(Win32ControllersTest, StoreLicensingIsUnsupportedOutsideAPackage) {
    // No package identity here. Every call must answer, not hang or throw.
    StoreLicensing store(QStringLiteral(" 9NTEST0000 "));
    EXPECT_EQ(store.proAddOnStoreId(), QStringLiteral("9NTEST0000"));
    EXPECT_FALSE(store.supported());

    QSignalSpy ready(&store, &StoreGateway::snapshotReady);
    store.refresh();
    ASSERT_EQ(ready.count(), 1);
    const StoreSnapshot snapshot = ready.at(0).at(0).value<StoreSnapshot>();
    EXPECT_FALSE(snapshot.available);
    EXPECT_FALSE(snapshot.addOnLicensed);
    EXPECT_FALSE(snapshot.acquired.isValid());
    EXPECT_EQ(snapshot.error, QStringLiteral("notPackaged"));

    QSignalSpy done(&store, &StoreGateway::purchaseFinished);
    store.purchase(QStringLiteral("9NTEST0000"));
    ASSERT_EQ(done.count(), 1);
    EXPECT_FALSE(done.at(0).at(1).toBool());
    EXPECT_EQ(done.at(0).at(2).toString(), mf::purchase::kUnavailable);

    store.purchase(QString());
    ASSERT_EQ(done.count(), 2);
    EXPECT_EQ(done.at(1).at(2).toString(), mf::purchase::kNotConfigured);
    store.setWindow(nullptr);
}

TEST(Win32ControllersTest, StoreLicensingMapsPurchaseStatuses) {
    using winrt::Windows::Services::Store::StorePurchaseStatus;
    bool ok = false;
    EXPECT_EQ(StoreLicensing::statusIdFor(StorePurchaseStatus::Succeeded, &ok), mf::purchase::kSucceeded);
    EXPECT_TRUE(ok);
    EXPECT_EQ(StoreLicensing::statusIdFor(StorePurchaseStatus::AlreadyPurchased, &ok), mf::purchase::kAlreadyPurchased);
    EXPECT_TRUE(ok);
    EXPECT_EQ(StoreLicensing::statusIdFor(StorePurchaseStatus::NotPurchased, &ok), mf::purchase::kNotPurchased);
    EXPECT_FALSE(ok);
    EXPECT_EQ(StoreLicensing::statusIdFor(StorePurchaseStatus::NetworkError, &ok), mf::purchase::kNetworkError);
    EXPECT_FALSE(ok);
    EXPECT_EQ(StoreLicensing::statusIdFor(StorePurchaseStatus::ServerError, &ok), mf::purchase::kServerError);
    EXPECT_FALSE(ok);
}

TEST(Win32ControllersTest, StoreLicensingMatchesAddOnKeysWithAndWithoutSkuSuffix) {
    const QString id = QStringLiteral("9NABCDEF1234");
    EXPECT_TRUE(StoreLicensing::licenseKeyMatches(id, id));
    EXPECT_TRUE(StoreLicensing::licenseKeyMatches(QStringLiteral("9nabcdef1234"), id));
    EXPECT_TRUE(StoreLicensing::licenseKeyMatches(QStringLiteral("9NABCDEF1234/0010"), id));
    EXPECT_FALSE(StoreLicensing::licenseKeyMatches(QStringLiteral("9NABCDEF12345"), id));
    EXPECT_FALSE(StoreLicensing::licenseKeyMatches(QStringLiteral("9NOTHER00000/0010"), id));
    EXPECT_FALSE(StoreLicensing::licenseKeyMatches(QString(), id));
    EXPECT_FALSE(StoreLicensing::licenseKeyMatches(id, QString()));
}

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

TEST(Win32ControllersTest, TaskbarBackdropStartsDarkAndToleratesNullWindow) {
    TaskbarBackdrop backdrop;
    EXPECT_FALSE(backdrop.known());
    EXPECT_FALSE(backdrop.light());
    EXPECT_LT(backdrop.luminance(), 0.0);
    backdrop.sample(nullptr);
    EXPECT_FALSE(backdrop.known());
}

TEST(Win32ControllersTest, TaskbarBackdropFlipsToneAndNotifiesOnce) {
    TaskbarBackdrop backdrop;
    int notified = 0;
    QObject::connect(&backdrop, &TaskbarBackdrop::changed, &backdrop, [&notified] { ++notified; });

    backdrop.applyLuma(0.1);
    EXPECT_TRUE(backdrop.known());
    EXPECT_FALSE(backdrop.light());
    EXPECT_EQ(notified, 1);

    backdrop.applyLuma(0.1); // No movement. No signal.
    EXPECT_EQ(notified, 1);

    for(int i = 0; i < 10 && !backdrop.light(); ++i)
        backdrop.applyLuma(1.0);
    EXPECT_TRUE(backdrop.light());
    EXPECT_GE(notified, 2);
}

TEST(Win32ControllersTest, TaskbarBackdropReadableKeepsHueAndAlpha) {
    TaskbarBackdrop backdrop;
    const QColor white("#FFFFFF");
    const QColor black("#000000");
    EXPECT_LT(backdrop.readable(white, true).lightnessF(), 0.5);
    EXPECT_GT(backdrop.readable(black, false).lightnessF(), 0.5);
    EXPECT_EQ(backdrop.readable(white, false), white);
    EXPECT_EQ(backdrop.readable(black, true), black);

    QColor accent("#7FD6FF");
    accent.setAlphaF(0.5f);
    const QColor onLight = backdrop.readable(accent, true);
    EXPECT_NEAR(onLight.hueF(), accent.hueF(), 0.01);
    EXPECT_NEAR(onLight.alphaF(), 0.5, 0.01);
    EXPECT_LT(onLight.lightnessF(), accent.lightnessF());

    EXPECT_FALSE(backdrop.readable(QColor(), true).isValid());
}

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
        EXPECT_LT(position.x() + width, screen.right() + 1) << "monitor " << i;
        EXPECT_GE(position.x(), screen.left()) << "monitor " << i;
        EXPECT_GT(position.x() + width, screen.left() + screen.width() / 2) << "monitor " << i;
    }
}

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
