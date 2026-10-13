#include <gtest/gtest.h>

#include "BatteryAlertPolicy.h"

using namespace mf;

namespace {

QVector<PeripheralSnapshot> snapshot(std::initializer_list<PeripheralSnapshot> devices) {
    return QVector<PeripheralSnapshot>(devices);
}

BatteryAlertPolicy seededWith(const QVector<PeripheralSnapshot>& devices) {
    BatteryAlertPolicy policy;
    policy.update(devices, 0);
    return policy;
}

bool contains(const QVector<PeripheralAlert>& alerts, PeripheralEvent event, const QString& name) {
    for(const PeripheralAlert& alert : alerts)
        if(alert.event == event && alert.deviceName == name)
            return true;
    return false;
}

} // namespace

TEST(BatteryAlertPolicyTest, MapsPercentagesToBuckets) {
    EXPECT_EQ(bucketOf(-1), BatteryBucket::Unknown);
    EXPECT_EQ(bucketOf(5), BatteryBucket::VeryLow);
    EXPECT_EQ(bucketOf(10), BatteryBucket::VeryLow);
    EXPECT_EQ(bucketOf(11), BatteryBucket::Low);
    EXPECT_EQ(bucketOf(20), BatteryBucket::Low);
    EXPECT_EQ(bucketOf(21), BatteryBucket::Half);
    EXPECT_EQ(bucketOf(40), BatteryBucket::Half);
    EXPECT_EQ(bucketOf(41), BatteryBucket::Good);
    EXPECT_EQ(bucketOf(89), BatteryBucket::Good);
    EXPECT_EQ(bucketOf(90), BatteryBucket::Full);
    EXPECT_EQ(bucketOf(100), BatteryBucket::Full);
}

TEST(BatteryAlertPolicyTest, FirstSnapshotOnlySeedsTheBaseline) {
    BatteryAlertPolicy policy;
    const auto alerts = policy.update(snapshot({{"Mouse", 55, false}, {"Headset", 80, false}}), 0);

    EXPECT_TRUE(alerts.isEmpty());
    EXPECT_TRUE(policy.hasBaseline());
}

TEST(BatteryAlertPolicyTest, ReportsConnectAndDisconnect) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 55, false}}));

    auto alerts = policy.update(snapshot({{"Mouse", 55, false}, {"Headset", 80, false}}), 10000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::Connected);
    EXPECT_EQ(alerts[0].deviceName, QStringLiteral("Headset"));

    EXPECT_TRUE(policy.update(snapshot({{"Mouse", 55, false}}), 20000).isEmpty());

    alerts = policy.update(snapshot({{"Mouse", 55, false}}), 24000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::Disconnected);
    EXPECT_EQ(alerts[0].deviceName, QStringLiteral("Headset"));
}

TEST(BatteryAlertPolicyTest, DisconnectCarriesTheLastKnownLevel) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Headset", 42, false}}));

    policy.update(snapshot({}), 10000);
    const auto alerts = policy.update(snapshot({}), 14000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].percent, 42);
    EXPECT_EQ(alerts[0].bucket, BatteryBucket::Good);
}

TEST(BatteryAlertPolicyTest, IgnoresASingleMissingSnapshot) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 55, false}}));

    EXPECT_TRUE(policy.update(snapshot({}), 10000).isEmpty());
    EXPECT_TRUE(policy.update(snapshot({{"Mouse", 55, false}}), 11000).isEmpty());
}

TEST(BatteryAlertPolicyTest, AlertsOnceWhenCrossingTheLowThreshold) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 25, false}}));

    auto alerts = policy.update(snapshot({{"Mouse", 19, false}}), 10000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::LowBattery);

    EXPECT_TRUE(policy.update(snapshot({{"Mouse", 19, false}}), 25000).isEmpty());
    EXPECT_TRUE(policy.update(snapshot({{"Mouse", 18, false}}), 40000).isEmpty());
}

TEST(BatteryAlertPolicyTest, EscalatesToVeryLowWithoutRepeatingTheLowAlert) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 25, false}}));
    policy.update(snapshot({{"Mouse", 19, false}}), 10000);

    const auto alerts = policy.update(snapshot({{"Mouse", 9, false}}), 20000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::VeryLowBattery);
    EXPECT_EQ(alerts[0].bucket, BatteryBucket::VeryLow);
}

TEST(BatteryAlertPolicyTest, GoingStraightToVeryLowSkipsTheLowAlert) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 55, false}}));

    const auto alerts = policy.update(snapshot({{"Mouse", 8, false}}), 10000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::VeryLowBattery);
}

TEST(BatteryAlertPolicyTest, ChargingBackUpReArmsTheThreshold) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 25, false}}));
    policy.update(snapshot({{"Mouse", 15, false}}), 10000);

    policy.update(snapshot({{"Mouse", 60, true}}), 20000);
    policy.update(snapshot({{"Mouse", 60, false}}), 30000);
    const auto alerts = policy.update(snapshot({{"Mouse", 15, false}}), 40000);

    EXPECT_TRUE(contains(alerts, PeripheralEvent::LowBattery, QStringLiteral("Mouse")));
}

TEST(BatteryAlertPolicyTest, DoesNotWarnWhileCharging) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 25, false}}));

    const auto alerts = policy.update(snapshot({{"Mouse", 8, true}}), 10000);
    EXPECT_FALSE(contains(alerts, PeripheralEvent::VeryLowBattery, QStringLiteral("Mouse")));
}

TEST(BatteryAlertPolicyTest, ReportsChargingTransitions) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Earbuds", 50, false}}));

    auto alerts = policy.update(snapshot({{"Earbuds", 50, true}}), 10000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::ChargingStarted);
    EXPECT_TRUE(alerts[0].charging);

    alerts = policy.update(snapshot({{"Earbuds", 55, false}}), 20000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::ChargingStopped);
    EXPECT_FALSE(alerts[0].charging);
}

TEST(BatteryAlertPolicyTest, ConnectAndDisconnectCarryTheChargingState) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 55, false}}));

    auto alerts = policy.update(snapshot({{"Mouse", 55, false}, {"Headset", 80, true}}), 10000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::Connected);
    EXPECT_TRUE(alerts[0].charging);

    policy.update(snapshot({{"Mouse", 55, false}}), 20000);
    alerts = policy.update(snapshot({{"Mouse", 55, false}}), 24000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::Disconnected);
    EXPECT_TRUE(alerts[0].charging);
}

TEST(BatteryAlertPolicyTest, DebouncesAFlappingBluetoothLink) {
    BatteryAlertPolicy policy = seededWith(snapshot({{"Mouse", 55, false}}));
    policy.setDebounceMs(3000);

    policy.update(snapshot({}), 10000);
    const auto alerts = policy.update(snapshot({{"Mouse", 55, false}}), 11000);
    EXPECT_TRUE(alerts.isEmpty());
}

TEST(BatteryAlertPolicyTest, DevicesWithoutABatteryStillReportConnection) {
    BatteryAlertPolicy policy = seededWith(snapshot({}));

    const auto alerts = policy.update(snapshot({{"Speaker", -1, false}}), 10000);
    ASSERT_EQ(alerts.size(), 1);
    EXPECT_EQ(alerts[0].event, PeripheralEvent::Connected);
    EXPECT_EQ(alerts[0].bucket, BatteryBucket::Unknown);
}
