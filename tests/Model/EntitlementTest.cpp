#include <gtest/gtest.h>

#include <QDateTime>
#include <QTimeZone>

#include "Entitlement.h"

using mf::EntitlementCache;
using mf::EntitlementInputs;
using mf::EntitlementSource;
using mf::EntitlementState;
using mf::InstallKind;
using mf::ProFeature;
using mf::StoreFacts;
using mf::Tier;

namespace {

QDateTime utc(int y, int mo, int d, int h = 0) {
    return QDateTime(QDate(y, mo, d), QTime(h, 0), QTimeZone::UTC);
}

const QDateTime kCutover = utc(2026, 12, 1);
const QDateTime kNow = utc(2027, 1, 20, 12);

// Store install, gating on, cutover set, Store silent, no cache.
EntitlementInputs storeBase() {
    EntitlementInputs in;
    in.install = InstallKind::Store;
    in.policy.cutover = kCutover;
    in.policy.trialDays = 15;
    in.now = kNow;
    return in;
}

StoreFacts answered(const QDateTime& acquired, bool addOn = false) {
    StoreFacts f;
    f.available = true;
    f.addOnLicensed = addOn;
    f.acquired = acquired;
    return f;
}

} // namespace

// --- fail open -------------------------------------------------------------

TEST(EntitlementTest, InstallerAndPortableAreAlwaysPro) {
    for(InstallKind kind : {InstallKind::Installer, InstallKind::Portable, InstallKind::Unknown}) {
        EntitlementInputs in = storeBase();
        in.install = kind;
        const EntitlementState s = mf::resolveEntitlement(in);
        EXPECT_EQ(s.tier, Tier::Pro);
        EXPECT_EQ(s.source, EntitlementSource::Unlocked);
    }
}

TEST(EntitlementTest, GatingOffUnlocksTheStoreBuild) {
    EntitlementInputs in = storeBase();
    in.policy.gatingEnabled = false;
    in.store = answered(utc(2027, 1, 1));
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Pro);
    EXPECT_EQ(s.source, EntitlementSource::Unlocked);
}

TEST(EntitlementTest, MissingCutoverUnlocksEverything) {
    EntitlementInputs in = storeBase();
    in.policy.cutover = QDateTime();
    in.store = answered(utc(2027, 1, 1));
    EXPECT_EQ(mf::resolveEntitlement(in).source, EntitlementSource::Unlocked);
}

TEST(EntitlementTest, InvalidClockWithNoCacheUnlocks) {
    EntitlementInputs in = storeBase();
    in.now = QDateTime();
    EXPECT_EQ(mf::resolveEntitlement(in).source, EntitlementSource::Unlocked);
}

// --- Store answered --------------------------------------------------------

TEST(EntitlementTest, AddOnLicenseIsPro) {
    EntitlementInputs in = storeBase();
    in.store = answered(utc(2027, 1, 1), true);
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Pro);
    EXPECT_EQ(s.source, EntitlementSource::AddOn);
    EXPECT_FALSE(s.trialEnd.isValid());
}

TEST(EntitlementTest, AddOnWinsOverAnExpiredTrial) {
    EntitlementInputs in = storeBase();
    in.store = answered(utc(2026, 12, 2), true);
    EXPECT_EQ(mf::resolveEntitlement(in).source, EntitlementSource::AddOn);
}

TEST(EntitlementTest, AcquiredBeforeCutoverIsGrandfathered) {
    EntitlementInputs in = storeBase();
    in.store = answered(utc(2025, 6, 1));
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Pro);
    EXPECT_EQ(s.source, EntitlementSource::Grandfathered);
}

TEST(EntitlementTest, AcquiredOneSecondBeforeCutoverIsGrandfathered) {
    EntitlementInputs in = storeBase();
    in.store = answered(kCutover.addSecs(-1));
    EXPECT_EQ(mf::resolveEntitlement(in).source, EntitlementSource::Grandfathered);
}

TEST(EntitlementTest, AcquiredAtCutoverIsNotGrandfathered) {
    EntitlementInputs in = storeBase();
    in.store = answered(kCutover);
    EXPECT_NE(mf::resolveEntitlement(in).source, EntitlementSource::Grandfathered);
}

TEST(EntitlementTest, StoreTrialRunsFifteenDaysFromAcquisition) {
    EntitlementInputs in = storeBase();
    const QDateTime acquired = kNow.addDays(-3);
    in.store = answered(acquired);
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Trial);
    EXPECT_EQ(s.source, EntitlementSource::StoreTrial);
    EXPECT_EQ(s.trialEnd, acquired.addDays(15));
    EXPECT_EQ(mf::trialDaysLeft(kNow, s.trialEnd), 12);
}

TEST(EntitlementTest, StoreTrialExpiresAfterFifteenDays) {
    EntitlementInputs in = storeBase();
    in.store = answered(kNow.addDays(-15));
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Free);
    EXPECT_EQ(s.source, EntitlementSource::Expired);
    EXPECT_EQ(s.trialEnd, kNow);
}

TEST(EntitlementTest, StoreTrialIsStillActiveOneSecondBeforeEnd) {
    EntitlementInputs in = storeBase();
    in.store = answered(kNow.addDays(-15).addSecs(1));
    EXPECT_EQ(mf::resolveEntitlement(in).tier, Tier::Trial);
}

TEST(EntitlementTest, StoreAnswerOverridesAStaleProCache) {
    // Refund case. The Store is the source of truth when it answers.
    EntitlementInputs in = storeBase();
    in.store = answered(kNow.addDays(-30));
    in.cache.valid = true;
    in.cache.tier = Tier::Pro;
    EXPECT_EQ(mf::resolveEntitlement(in).tier, Tier::Free);
}

TEST(EntitlementTest, StoreAnswerWithoutAcquisitionDateFallsBackToCache) {
    EntitlementInputs in = storeBase();
    in.store = answered(QDateTime());
    in.cache.valid = true;
    in.cache.tier = Tier::Pro;
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Pro);
    EXPECT_EQ(s.source, EntitlementSource::Cached);
}

TEST(EntitlementTest, StoreAnswerWithoutAcquisitionDateAndNoCacheStartsLocalTrial) {
    EntitlementInputs in = storeBase();
    in.store = answered(QDateTime());
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Trial);
    EXPECT_EQ(s.source, EntitlementSource::LocalTrial);
    EXPECT_EQ(s.trialEnd, kNow.addDays(15));
}

// --- Store silent ----------------------------------------------------------

TEST(EntitlementTest, OfflineWithCachedProStaysPro) {
    EntitlementInputs in = storeBase();
    in.cache.valid = true;
    in.cache.tier = Tier::Pro;
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Pro);
    EXPECT_EQ(s.source, EntitlementSource::Cached);
}

TEST(EntitlementTest, OfflineWithCachedTrialEndKeepsTheSameEnd) {
    EntitlementInputs in = storeBase();
    in.cache.valid = true;
    in.cache.tier = Tier::Trial;
    in.cache.trialEnd = kNow.addDays(2);
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Trial);
    EXPECT_EQ(s.source, EntitlementSource::Cached);
    EXPECT_EQ(s.trialEnd, kNow.addDays(2));
}

TEST(EntitlementTest, OfflineWithCachedTrialEndInThePastIsExpired) {
    EntitlementInputs in = storeBase();
    in.cache.valid = true;
    in.cache.tier = Tier::Trial;
    in.cache.trialEnd = kNow.addSecs(-1);
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_EQ(s.tier, Tier::Free);
    EXPECT_EQ(s.source, EntitlementSource::Expired);
}

TEST(EntitlementTest, OfflineFirstRunStartsALocalTrial) {
    const EntitlementState s = mf::resolveEntitlement(storeBase());
    EXPECT_EQ(s.tier, Tier::Trial);
    EXPECT_EQ(s.source, EntitlementSource::LocalTrial);
    EXPECT_EQ(s.trialEnd, kNow.addDays(15));
    EXPECT_EQ(mf::trialDaysLeft(kNow, s.trialEnd), 15);
}

TEST(EntitlementTest, ZeroTrialDaysExpiresAtOnce) {
    EntitlementInputs in = storeBase();
    in.policy.trialDays = 0;
    EXPECT_EQ(mf::resolveEntitlement(in).tier, Tier::Free);
}

// --- clock -----------------------------------------------------------------

TEST(EntitlementTest, ClockRollbackCannotExtendATrial) {
    EntitlementInputs in = storeBase();
    in.cache.valid = true;
    in.cache.tier = Tier::Trial;
    in.cache.trialEnd = kNow.addDays(1);
    in.cache.lastSeen = kNow.addDays(5); // the app already ran past the end
    in.now = kNow;                        // then the clock was set back
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_TRUE(s.clockRolledBack);
    EXPECT_EQ(s.effectiveNow, kNow.addDays(5));
    EXPECT_EQ(s.tier, Tier::Free);
}

TEST(EntitlementTest, ClockRollbackAlsoAppliesToStoreTrials) {
    EntitlementInputs in = storeBase();
    in.store = answered(kNow.addDays(-10));
    in.cache.valid = true;
    in.cache.tier = Tier::Trial;
    in.cache.lastSeen = kNow.addDays(10);
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_TRUE(s.clockRolledBack);
    EXPECT_EQ(s.tier, Tier::Free);
}

TEST(EntitlementTest, ForwardClockIsNotARollback) {
    EntitlementInputs in = storeBase();
    in.cache.valid = true;
    in.cache.tier = Tier::Trial;
    in.cache.trialEnd = kNow.addDays(1);
    in.cache.lastSeen = kNow.addDays(-1);
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_FALSE(s.clockRolledBack);
    EXPECT_EQ(s.effectiveNow, kNow);
    EXPECT_EQ(s.tier, Tier::Trial);
}

TEST(EntitlementTest, InvalidClockUsesLastSeen) {
    EntitlementInputs in = storeBase();
    in.now = QDateTime();
    in.cache.valid = true;
    in.cache.tier = Tier::Trial;
    in.cache.trialEnd = kNow.addDays(1);
    in.cache.lastSeen = kNow;
    const EntitlementState s = mf::resolveEntitlement(in);
    EXPECT_FALSE(s.clockRolledBack);
    EXPECT_EQ(s.effectiveNow, kNow);
    EXPECT_EQ(s.tier, Tier::Trial);
}

TEST(EntitlementTest, CacheFromStateKeepsLastSeenMonotonic) {
    EntitlementInputs in = storeBase();
    in.cache.valid = true;
    in.cache.tier = Tier::Trial;
    in.cache.trialEnd = kNow.addDays(20);
    in.cache.lastSeen = kNow.addDays(5);
    const EntitlementCache c = mf::cacheFrom(mf::resolveEntitlement(in));
    EXPECT_TRUE(c.valid);
    EXPECT_EQ(c.tier, Tier::Trial);
    EXPECT_EQ(c.trialEnd, kNow.addDays(20));
    EXPECT_EQ(c.lastSeen, kNow.addDays(5));
}

TEST(EntitlementTest, CacheRoundTripResolvesToTheSameState) {
    EntitlementInputs first = storeBase();
    const EntitlementState a = mf::resolveEntitlement(first);
    EntitlementInputs second = storeBase();
    second.cache = mf::cacheFrom(a);
    second.now = kNow.addDays(1);
    const EntitlementState b = mf::resolveEntitlement(second);
    EXPECT_EQ(b.tier, Tier::Trial);
    EXPECT_EQ(b.source, EntitlementSource::Cached);
    EXPECT_EQ(b.trialEnd, a.trialEnd);
}

// --- helpers ---------------------------------------------------------------

TEST(EntitlementTest, TrialAndProUnlockEveryFeature) {
    for(ProFeature f : {ProFeature::Visualizer, ProFeature::PerAppMixer, ProFeature::PeripheralBattery,
                        ProFeature::NowPlayingStyles, ProFeature::TaskbarWidgetOptions}) {
        EXPECT_TRUE(mf::isUnlocked(f, Tier::Pro));
        EXPECT_TRUE(mf::isUnlocked(f, Tier::Trial));
        EXPECT_FALSE(mf::isUnlocked(f, Tier::Free));
    }
    EXPECT_FALSE(mf::isUnlocked(Tier::Free));
    EXPECT_TRUE(mf::isUnlocked(Tier::Trial));
}

TEST(EntitlementTest, TrialDaysLeftRoundsUp) {
    EXPECT_EQ(mf::trialDaysLeft(kNow, kNow.addSecs(1)), 1);
    EXPECT_EQ(mf::trialDaysLeft(kNow, kNow.addDays(1)), 1);
    EXPECT_EQ(mf::trialDaysLeft(kNow, kNow.addDays(1).addSecs(1)), 2);
    EXPECT_EQ(mf::trialDaysLeft(kNow, kNow), 0);
    EXPECT_EQ(mf::trialDaysLeft(kNow, kNow.addDays(-1)), 0);
    EXPECT_EQ(mf::trialDaysLeft(kNow, QDateTime()), 0);
    EXPECT_EQ(mf::trialDaysLeft(QDateTime(), kNow), 0);
}

TEST(EntitlementTest, IdsAreStable) {
    EXPECT_EQ(mf::tierId(Tier::Free), QStringLiteral("free"));
    EXPECT_EQ(mf::tierId(Tier::Trial), QStringLiteral("trial"));
    EXPECT_EQ(mf::tierId(Tier::Pro), QStringLiteral("pro"));
    EXPECT_EQ(mf::sourceId(EntitlementSource::Grandfathered), QStringLiteral("grandfathered"));
    EXPECT_EQ(mf::sourceId(EntitlementSource::Expired), QStringLiteral("expired"));
    EXPECT_EQ(mf::featureId(ProFeature::Visualizer), QStringLiteral("visualizer"));
    EXPECT_EQ(mf::featureId(ProFeature::PerAppMixer), QStringLiteral("perAppMixer"));
    EXPECT_EQ(mf::featureId(ProFeature::PeripheralBattery), QStringLiteral("peripheralBattery"));
    EXPECT_EQ(mf::featureId(ProFeature::NowPlayingStyles), QStringLiteral("nowPlayingStyles"));
    EXPECT_EQ(mf::featureId(ProFeature::TaskbarWidgetOptions), QStringLiteral("taskbarWidgetOptions"));
}

TEST(EntitlementTest, FeatureIdsRoundTrip) {
    const QStringList ids = mf::allFeatureIds();
    EXPECT_EQ(ids.size(), 5);
    for(const QString& id : ids) {
        ProFeature f = ProFeature::Visualizer;
        ASSERT_TRUE(mf::featureFromId(id, &f)) << id.toStdString();
        EXPECT_EQ(mf::featureId(f), id);
    }
    ProFeature untouched = ProFeature::PerAppMixer;
    EXPECT_FALSE(mf::featureFromId(QStringLiteral("nope"), &untouched));
    EXPECT_EQ(untouched, ProFeature::PerAppMixer);
    EXPECT_FALSE(mf::featureFromId(QStringLiteral("Visualizer"), nullptr));
}

TEST(EntitlementTest, DescribeMentionsTierSourceAndDays) {
    EntitlementInputs in = storeBase();
    in.store = answered(kNow.addDays(-3));
    const QString text = mf::describeEntitlement(mf::resolveEntitlement(in));
    EXPECT_TRUE(text.contains(QStringLiteral("tier=trial"))) << text.toStdString();
    EXPECT_TRUE(text.contains(QStringLiteral("source=storeTrial"))) << text.toStdString();
    EXPECT_TRUE(text.contains(QStringLiteral("daysLeft=12"))) << text.toStdString();
    EXPECT_TRUE(text.contains(QStringLiteral("clockRolledBack=false"))) << text.toStdString();
}

TEST(EntitlementTest, DescribeWithoutTrialSaysNone) {
    EntitlementInputs in = storeBase();
    in.install = InstallKind::Portable;
    const QString text = mf::describeEntitlement(mf::resolveEntitlement(in));
    EXPECT_TRUE(text.contains(QStringLiteral("trialEnd=none"))) << text.toStdString();
}
