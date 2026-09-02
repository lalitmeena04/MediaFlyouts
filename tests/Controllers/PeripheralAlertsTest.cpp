#include <gtest/gtest.h>

#include <functional>
#include <memory>

#include "BatteryProviders/IBatteryProvider.h"
#include "BatteryStatusController.h"
#include "PeripheralAlerts.h"

namespace {

class FakeProvider : public IBatteryProvider {
    public:
        explicit FakeProvider(std::function<QVector<BatteryDeviceInfo>()> poller) : m_poller(std::move(poller)) {}
        QVector<BatteryDeviceInfo> poll() override { return m_poller(); }

    private:
        std::function<QVector<BatteryDeviceInfo>()> m_poller;
};

std::vector<std::unique_ptr<IBatteryProvider>> providersOf(std::function<QVector<BatteryDeviceInfo>()> poller) {
    std::vector<std::unique_ptr<IBatteryProvider>> providers;
    providers.push_back(std::make_unique<FakeProvider>(std::move(poller)));
    return providers;
}

QString eventOf(const QVariantList& alerts, int index) {
    return alerts.at(index).toMap().value(QStringLiteral("event")).toString();
}

} // namespace

TEST(PeripheralAlertsTest, StaysQuietForDevicesAlreadyConnectedAtStartup) {
    BatteryStatusController status(
        providersOf([] { return QVector<BatteryDeviceInfo>{{"Headset", 80, "bluetooth", "headset"}}; }));
    PeripheralAlerts alerts(&status);

    EXPECT_TRUE(alerts.pending().isEmpty());
}

TEST(PeripheralAlertsTest, RaisesAnAlertWhenADeviceAppears) {
    QVector<BatteryDeviceInfo> devices{{"Headset", 80, "bluetooth", "headset"}};
    BatteryStatusController status(providersOf([&devices] { return devices; }));
    PeripheralAlerts alerts(&status);

    int raised = 0;
    QObject::connect(&alerts, &PeripheralAlerts::alertsRaised, [&] { ++raised; });

    devices.append({"Mouse", 53, "wireless", "mouse"});
    status.refresh();

    ASSERT_EQ(alerts.pending().size(), 1);
    EXPECT_EQ(eventOf(alerts.pending(), 0), QStringLiteral("connected"));
    EXPECT_EQ(alerts.pending().at(0).toMap().value(QStringLiteral("name")).toString(), QStringLiteral("Mouse"));
    EXPECT_EQ(raised, 1);
}

TEST(PeripheralAlertsTest, DefersAnAlertWhenADeviceFirstGoesAway) {
    QVector<BatteryDeviceInfo> devices{{"Headset", 80, "bluetooth", "headset"},
                                       {"Mouse", 53, "wireless", "mouse"}};
    BatteryStatusController status(providersOf([&devices] { return devices; }));
    PeripheralAlerts alerts(&status);

    devices.removeLast();
    status.refresh();

    EXPECT_TRUE(alerts.pending().isEmpty());
}

TEST(PeripheralAlertsTest, ReportsSeveralDevicesFromOneSnapshot) {
    QVector<BatteryDeviceInfo> devices{{"Headset", 80, "bluetooth", "headset"}};
    BatteryStatusController status(providersOf([&devices] { return devices; }));
    PeripheralAlerts alerts(&status);

    // two devices connect between polls: the flyout shows them side by side
    devices.append({"Mouse", 53, "wireless", "mouse"});
    devices.append({"Keyboard", 90, "wireless", "keyboard"});
    status.refresh();

    EXPECT_EQ(alerts.pending().size(), 2);
}

TEST(PeripheralAlertsTest, WarnsOnceWhenTheBatteryCrossesTheLowThreshold) {
    QVector<BatteryDeviceInfo> devices{{"Mouse", 25, "wireless", "mouse"}};
    BatteryStatusController status(providersOf([&devices] { return devices; }));
    PeripheralAlerts alerts(&status);

    devices[0].percent = 18;
    status.refresh();
    ASSERT_EQ(alerts.pending().size(), 1);
    EXPECT_EQ(eventOf(alerts.pending(), 0), QStringLiteral("lowBattery"));

    // the next poll still sees a low battery but must not warn again
    alerts.clear();
    devices[0].percent = 17;
    status.refresh();
    EXPECT_TRUE(alerts.pending().isEmpty());
}

TEST(PeripheralAlertsTest, ReportsChargingTransitions) {
    QVector<BatteryDeviceInfo> devices{{"Earbuds", 50, "bluetooth", "headset"}};
    BatteryStatusController status(providersOf([&devices] { return devices; }));
    PeripheralAlerts alerts(&status);

    devices[0].charging = true;
    status.refresh();

    ASSERT_EQ(alerts.pending().size(), 1);
    EXPECT_EQ(eventOf(alerts.pending(), 0), QStringLiteral("chargingStarted"));
}

TEST(PeripheralAlertsTest, ExposesBucketLabels) {
    EXPECT_EQ(PeripheralAlerts::bucketLabel(95), QStringLiteral("Full"));
    EXPECT_EQ(PeripheralAlerts::bucketLabel(30), QStringLiteral("Half"));
    EXPECT_EQ(PeripheralAlerts::bucketLabel(15), QStringLiteral("Low"));
    EXPECT_EQ(PeripheralAlerts::bucketLabel(5), QStringLiteral("Very low"));
    EXPECT_EQ(PeripheralAlerts::bucketLabel(-1), QStringLiteral("Unknown"));
}

TEST(PeripheralAlertsTest, CarriesTheDeviceTypeAndTransportToTheCard) {
    // without these the flyout falls back to a name guess and shows the wrong icon
    QVector<BatteryDeviceInfo> devices{{"Headset", 80, "bluetooth", "headset"}};
    BatteryStatusController status(providersOf([&devices] { return devices; }));
    PeripheralAlerts alerts(&status);

    devices.append({"Razer Basilisk V3 X", 51, "wireless", "mouse"});
    status.refresh();

    ASSERT_EQ(alerts.pending().size(), 1);
    const QVariantMap alert = alerts.pending().at(0).toMap();
    EXPECT_EQ(alert.value(QStringLiteral("category")).toString(), QStringLiteral("mouse"));
    EXPECT_EQ(alert.value(QStringLiteral("connectionKind")).toString(), QStringLiteral("wireless"));
}

TEST(PeripheralAlertsTest, ClearingEmptiesThePendingList) {
    QVector<BatteryDeviceInfo> devices{{"Headset", 80, "bluetooth", "headset"}};
    BatteryStatusController status(providersOf([&devices] { return devices; }));
    PeripheralAlerts alerts(&status);

    devices.append({"Mouse", 53, "wireless", "mouse"});
    status.refresh();
    ASSERT_FALSE(alerts.pending().isEmpty());

    alerts.clear();
    EXPECT_TRUE(alerts.pending().isEmpty());
}
