#include <gtest/gtest.h>

#include <QDateTime>
#include <QTimeZone>

#include "ProBuildConfig.h"

using mf::ProBuildConfig;

TEST(ProBuildConfigTest, EmptyValuesMeanGatingOffAndNoPurchase) {
    const ProBuildConfig c = mf::parseProBuildConfig(QString(), QString(), QString(), QString());
    EXPECT_TRUE(c.proAddOnStoreId.isEmpty());
    EXPECT_FALSE(c.purchaseConfigured());
    EXPECT_FALSE(c.policy.cutover.isValid());
    EXPECT_TRUE(c.policy.gatingEnabled);
    EXPECT_EQ(c.policy.trialDays, 15);
    EXPECT_TRUE(c.tipAddOnStoreIds.isEmpty());
}

TEST(ProBuildConfigTest, ParsesEveryField) {
    const ProBuildConfig c = mf::parseProBuildConfig(
        QStringLiteral(" 9NABCDEF1234 "), QStringLiteral("2026-12-01T10:30:00Z"), QStringLiteral("30"),
        QStringLiteral("9NTIP0001, 9NTIP0002,,9NTIP0001"));
    EXPECT_EQ(c.proAddOnStoreId, QStringLiteral("9NABCDEF1234"));
    EXPECT_TRUE(c.purchaseConfigured());
    EXPECT_EQ(c.policy.cutover, QDateTime(QDate(2026, 12, 1), QTime(10, 30), QTimeZone::UTC));
    EXPECT_EQ(c.policy.cutover.timeSpec(), Qt::UTC);
    EXPECT_EQ(c.policy.trialDays, 30);
    EXPECT_EQ(c.tipAddOnStoreIds, (QStringList{QStringLiteral("9NTIP0001"), QStringLiteral("9NTIP0002")}));
}

TEST(ProBuildConfigTest, CutoverWithoutZoneIsReadAsUtc) {
    const ProBuildConfig c =
        mf::parseProBuildConfig(QString(), QStringLiteral("2026-12-01T10:30:00"), QString(), QString());
    EXPECT_EQ(c.policy.cutover, QDateTime(QDate(2026, 12, 1), QTime(10, 30), QTimeZone::UTC));
}

TEST(ProBuildConfigTest, CutoverWithOffsetIsNormalisedToUtc) {
    const ProBuildConfig c =
        mf::parseProBuildConfig(QString(), QStringLiteral("2026-12-01T16:00:00+05:30"), QString(), QString());
    EXPECT_EQ(c.policy.cutover, QDateTime(QDate(2026, 12, 1), QTime(10, 30), QTimeZone::UTC));
}

TEST(ProBuildConfigTest, BadCutoverLeavesGatingOff) {
    const ProBuildConfig c = mf::parseProBuildConfig(QString(), QStringLiteral("next tuesday"), QString(), QString());
    EXPECT_FALSE(c.policy.cutover.isValid());
}

TEST(ProBuildConfigTest, BadTrialDaysFallBackToFifteen) {
    EXPECT_EQ(mf::parseProBuildConfig(QString(), QString(), QStringLiteral("abc"), QString()).policy.trialDays, 15);
    EXPECT_EQ(mf::parseProBuildConfig(QString(), QString(), QStringLiteral("-3"), QString()).policy.trialDays, 15);
    EXPECT_EQ(mf::parseProBuildConfig(QString(), QString(), QStringLiteral("0"), QString()).policy.trialDays, 0);
}

TEST(ProBuildConfigTest, CompiledConfigIsReadable) {
    const ProBuildConfig c = mf::proBuildConfig();
    EXPECT_GE(c.policy.trialDays, 0);
    // Same object every time.
    EXPECT_EQ(mf::proBuildConfig().proAddOnStoreId, c.proAddOnStoreId);
}
