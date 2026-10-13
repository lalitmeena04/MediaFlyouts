#include <gtest/gtest.h>

#include <functional>

#include "BatteryProviders/IBatteryProvider.h"
#include "BatteryStatusController.h"

namespace {

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

TEST(BatteryStatusControllerTest, ExposesUnknownBatteryDevicesForConnectionAlerts) {
    BatteryStatusController controller(
        providersOf([] { return QVector<BatteryDeviceInfo>{{"AULA wireless keyboard", -1, "wireless", "keyboard"}}; }));

    const QVariantMap device = controller.devices().at(0).toMap();
    EXPECT_EQ(device[QStringLiteral("name")].toString(), QStringLiteral("AULA wireless keyboard"));
    EXPECT_EQ(device[QStringLiteral("percent")].toInt(), -1);
    EXPECT_EQ(device[QStringLiteral("connectionKind")].toString(), QStringLiteral("wireless"));
    EXPECT_EQ(device[QStringLiteral("category")].toString(), QStringLiteral("keyboard"));
}

TEST(BatteryStatusControllerTest, RefreshUpdatesDevicesAndEmitsSignalOnChange) {
    int percent = 40;
    BatteryStatusController controller(
        providersOf([&percent] { return QVector<BatteryDeviceInfo>{{"Headset", percent, "bluetooth"}}; }));

    int changedCount = 0;
    QObject::connect(&controller, &BatteryStatusController::devicesChanged, [&] { ++changedCount; });

    controller.refresh();
    EXPECT_EQ(changedCount, 0);

    percent = 35;
    controller.refresh();
    EXPECT_EQ(changedCount, 1);
    EXPECT_EQ(controller.devices()[0].toMap()[QStringLiteral("percent")].toInt(), 35);
}

TEST(BatteryStatusControllerTest, KeepsOnlyTheFirstProviderReportingADevice) {
    std::vector<std::unique_ptr<IBatteryProvider>> providers;
    providers.push_back(std::make_unique<FakeProvider>([] {
        BatteryDeviceInfo fastPair{"Ear (2)", 80, "bluetooth", "headset"};
        fastPair.components = {{"L", 100, false}, {"R", 80, false}, {"Case", 100, true}};
        return QVector<BatteryDeviceInfo>{fastPair};
    }));
    providers.push_back(std::make_unique<FakeProvider>(
        [] { return QVector<BatteryDeviceInfo>{{"Ear (2)", 60, "bluetooth", "headset"}}; }));

    BatteryStatusController controller(std::move(providers));
    const QVariantList devices = controller.devices();

    ASSERT_EQ(devices.size(), 1);
    EXPECT_EQ(devices[0].toMap()[QStringLiteral("percent")].toInt(), 80);
}

TEST(BatteryStatusControllerTest, ExposesComponentsAndChargingToQml) {
    BatteryStatusController controller(providersOf([] {
        BatteryDeviceInfo earbuds{"Ear (2)", 80, "bluetooth", "headset"};
        earbuds.charging = true;
        earbuds.components = {{"L", 100, false}, {"R", 80, false}, {"Case", 100, true}};
        return QVector<BatteryDeviceInfo>{earbuds};
    }));

    const QVariantMap device = controller.devices().at(0).toMap();
    EXPECT_TRUE(device[QStringLiteral("charging")].toBool());

    const QVariantList components = device[QStringLiteral("components")].toList();
    ASSERT_EQ(components.size(), 3);
    EXPECT_EQ(components[0].toMap()[QStringLiteral("label")].toString(), QStringLiteral("L"));
    EXPECT_EQ(components[1].toMap()[QStringLiteral("percent")].toInt(), 80);
    EXPECT_TRUE(components[2].toMap()[QStringLiteral("charging")].toBool());
}

TEST(BatteryStatusControllerTest, SingleBatteryDevicesReportNoComponents) {
    BatteryStatusController controller(
        providersOf([] { return QVector<BatteryDeviceInfo>{{"Mouse", 53, "wireless", "mouse"}}; }));

    const QVariantMap device = controller.devices().at(0).toMap();
    EXPECT_TRUE(device[QStringLiteral("components")].toList().isEmpty());
    EXPECT_FALSE(device[QStringLiteral("charging")].toBool());
    EXPECT_EQ(device[QStringLiteral("connectionKind")].toString(), QStringLiteral("wireless"));
}
