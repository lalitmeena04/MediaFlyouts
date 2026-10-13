#include <gtest/gtest.h>

#include "PowerAlertPolicy.h"

namespace {

mf::PowerSnapshot snapshot(mf::PowerSource source, int percent) {
    mf::PowerSnapshot s;
    s.source = source;
    s.percent = percent;
    return s;
}

} // namespace

TEST(PowerAlertPolicyTest, FirstReadingRaisesNoAlert) {
    mf::PowerAlertPolicy policy;

    EXPECT_FALSE(policy.hasBaseline());
    EXPECT_FALSE(policy.update(snapshot(mf::PowerSource::AC, 80)).has_value());
    EXPECT_TRUE(policy.hasBaseline());
}

TEST(PowerAlertPolicyTest, ReportsPlugInAndUnplug) {
    mf::PowerAlertPolicy policy;
    policy.update(snapshot(mf::PowerSource::Battery, 55));

    const std::optional<mf::ChargerAlert> plugged = policy.update(snapshot(mf::PowerSource::AC, 56));
    ASSERT_TRUE(plugged.has_value());
    EXPECT_EQ(plugged->event, mf::ChargerEvent::PluggedIn);
    EXPECT_EQ(plugged->percent, 56);

    const std::optional<mf::ChargerAlert> unplugged = policy.update(snapshot(mf::PowerSource::Battery, 54));
    ASSERT_TRUE(unplugged.has_value());
    EXPECT_EQ(unplugged->event, mf::ChargerEvent::Unplugged);
    EXPECT_EQ(unplugged->percent, 54);
}

TEST(PowerAlertPolicyTest, RepeatedSameSourceRaisesNoAlert) {
    mf::PowerAlertPolicy policy;
    policy.update(snapshot(mf::PowerSource::Battery, 40));

    EXPECT_FALSE(policy.update(snapshot(mf::PowerSource::Battery, 39)).has_value());
    EXPECT_FALSE(policy.update(snapshot(mf::PowerSource::Battery, 38)).has_value());
}

TEST(PowerAlertPolicyTest, UnknownReadingKeepsTheLastSource) {
    mf::PowerAlertPolicy policy;
    policy.update(snapshot(mf::PowerSource::AC, 90));

    EXPECT_FALSE(policy.update(snapshot(mf::PowerSource::Unknown, -1)).has_value());
    EXPECT_FALSE(policy.update(snapshot(mf::PowerSource::AC, 91)).has_value());

    const std::optional<mf::ChargerAlert> alert = policy.update(snapshot(mf::PowerSource::Battery, 91));
    ASSERT_TRUE(alert.has_value());
    EXPECT_EQ(alert->event, mf::ChargerEvent::Unplugged);
}

TEST(PowerAlertPolicyTest, UnknownFirstReadingLeavesNoBaseline) {
    mf::PowerAlertPolicy policy;

    EXPECT_FALSE(policy.update(snapshot(mf::PowerSource::Unknown, -1)).has_value());
    EXPECT_FALSE(policy.hasBaseline());
}

TEST(PowerAlertPolicyTest, AlertCarriesTheChargeBucket) {
    mf::PowerAlertPolicy policy;
    policy.update(snapshot(mf::PowerSource::AC, 8));

    const std::optional<mf::ChargerAlert> alert = policy.update(snapshot(mf::PowerSource::Battery, 8));
    ASSERT_TRUE(alert.has_value());
    EXPECT_EQ(alert->bucket, mf::bucketOf(8));
}

TEST(PowerAlertPolicyTest, UnknownPercentStaysUnknown) {
    mf::PowerAlertPolicy policy;
    policy.update(snapshot(mf::PowerSource::Battery, -1));

    const std::optional<mf::ChargerAlert> alert = policy.update(snapshot(mf::PowerSource::AC, -1));
    ASSERT_TRUE(alert.has_value());
    EXPECT_EQ(alert->percent, -1);
    EXPECT_EQ(alert->bucket, mf::BatteryBucket::Unknown);
}
