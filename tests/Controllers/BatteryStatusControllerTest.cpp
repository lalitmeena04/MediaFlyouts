#include <gtest/gtest.h>

#include <functional>

#include "BatteryProviders/IBatteryProvider.h"
#include "BatteryStatusController.h"

namespace {

// Provider stub whose poll() result is supplied by the test, so refresh() behavior
// can be exercised without touching real Bluetooth/BLE/gamepad APIs.
class FakeProvider : public IBatteryProvider {
    public:
        explicit FakeProvider(std::function<QVector<BatteryDeviceInfo>()> poller) : m_poller(std::move(poller)) {}

        QVector<BatteryDeviceInfo> poll() override {
            return m_poller();
        }

    private:
        std::function<QVector<BatteryDeviceInfo>()> m_poller;
};

std::vector<std::unique_ptr<IBatteryProvider>> providersOf(std::function<QVector<BatteryDeviceInfo>()> poller) {
    std::vector<std::unique_ptr<IBatteryProvider>> providers;
    providers.push_back(std::make_unique<FakeProvider>(std::move(poller)));
    return providers;
}

} // namespace

TEST(BatteryStatusControllerTest, AggregatesDevicesFromAllProviders) {
    std::vector<std::unique_ptr<IBatteryProvider>> providers;
    providers.push_back(std::make_unique<FakeProvider>(
        [] { return QVector<BatteryDeviceInfo>{{"Headset", 40, "bluetooth"}}; }));
    providers.push_back(
        std::make_unique<FakeProvider>([] { return QVector<BatteryDeviceInfo>{{"Keyboard", 70, "ble"}}; }));

    BatteryStatusController controller(std::move(providers));
    const QVariantList devices = controller.devices();

    ASSERT_EQ(devices.size(), 2);
    EXPECT_EQ(devices[0].toMap()[QStringLiteral("name")].toString(), QStringLiteral("Headset"));
    EXPECT_EQ(devices[0].toMap()[QStringLiteral("percent")].toInt(), 40);
    EXPECT_EQ(devices[1].toMap()[QStringLiteral("name")].toString(), QStringLiteral("Keyboard"));
}

TEST(BatteryStatusControllerTest, EmptyWhenNoProviderReportsAnything) {
    BatteryStatusController controller(providersOf([] { return QVector<BatteryDeviceInfo>{}; }));
    EXPECT_TRUE(controller.devices().isEmpty());
}

TEST(BatteryStatusControllerTest, RefreshUpdatesDevicesAndEmitsSignalOnChange) {
    int percent = 40;
    BatteryStatusController controller(
        providersOf([&percent] { return QVector<BatteryDeviceInfo>{{"Headset", percent, "bluetooth"}}; }));

    int changedCount = 0;
    QObject::connect(&controller, &BatteryStatusController::devicesChanged, [&] { ++changedCount; });

    // same reading again: no observable change, no signal
    controller.refresh();
    EXPECT_EQ(changedCount, 0);

    // battery level actually changed: refresh should pick it up and notify
    percent = 35;
    controller.refresh();
    EXPECT_EQ(changedCount, 1);
    EXPECT_EQ(controller.devices()[0].toMap()[QStringLiteral("percent")].toInt(), 35);
}
