#include <gtest/gtest.h>

#include <memory>

#include <QDateTime>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QWindow>

#include "EntitlementController.h"
#include "EntitlementStore.h"
#include "StoreGateway.h"
#include "TestSupport/IsolatedSettings.h"

using mf::EntitlementSource;
using mf::InstallKind;
using mf::ProBuildConfig;
using mf::Tier;

namespace {

class FakeStoreGateway : public StoreGateway {
        Q_OBJECT
    public:
        bool supportedFlag = true;
        int refreshCalls = 0;
        QStringList purchases;
        QWindow* window = nullptr;

        bool supported() const override { return supportedFlag; }
        void setWindow(QWindow* w) override { window = w; }
        void refresh() override { ++refreshCalls; }
        void purchase(const QString& storeId) override { purchases << storeId; }

        void answer(const StoreSnapshot& s) { emit snapshotReady(s); }
        void finishPurchase(const QString& id, bool ok, const QString& status) { emit purchaseFinished(id, ok, status); }
        void changeLicenses() { emit licensesChanged(); }
};

const QDateTime kCutover(QDate(2026, 12, 1), QTime(0, 0), QTimeZone::UTC);
const QDateTime kNow(QDate(2027, 1, 20), QTime(12, 0), QTimeZone::UTC);
const QString kProId = QStringLiteral("9NPRO000");

ProBuildConfig gatedConfig() {
    ProBuildConfig c;
    c.proAddOnStoreId = kProId;
    c.policy.cutover = kCutover;
    c.policy.trialDays = 15;
    c.tipAddOnStoreIds = {QStringLiteral("9NTIP001")};
    return c;
}

StoreSnapshot storeAnswer(const QDateTime& acquired, bool addOn = false, const QString& price = QString()) {
    StoreSnapshot s;
    s.available = true;
    s.appLicenseActive = true;
    s.addOnLicensed = addOn;
    s.acquired = acquired;
    s.proPrice = price;
    return s;
}

class EntitlementControllerTest : public ::testing::Test {
    protected:
        void SetUp() override {
            mftest::isolateUserSettings();
            ASSERT_TRUE(m_dir.isValid());
            m_now = kNow;
        }

        std::unique_ptr<mf::EntitlementStore> store() {
            return std::make_unique<mf::EntitlementStore>(m_dir.filePath(QStringLiteral("Entitlement.dat")),
                                                          std::make_unique<mf::ChecksumCodec>());
        }

        std::unique_ptr<EntitlementController> make(InstallKind kind, const ProBuildConfig& config,
                                                    FakeStoreGateway* gateway) {
            auto c = std::make_unique<EntitlementController>(kind, config, store());
            c->setClock([this] { return m_now; });
            c->setGateway(gateway);
            return c;
        }

        QTemporaryDir m_dir;
        QDateTime m_now;
};

} // namespace

// --- fail open -------------------------------------------------------------

TEST_F(EntitlementControllerTest, InstallerBuildIsProButStillLogsStoreFacts) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Installer, gatedConfig(), &gateway);
    c->start();
    EXPECT_TRUE(c->isPro());
    EXPECT_TRUE(c->unlocked());
    EXPECT_EQ(c->sourceId(), QStringLiteral("unlocked"));
    EXPECT_FALSE(c->canPurchase());
    // Store facts are still read, for the logs.
    EXPECT_EQ(gateway.refreshCalls, 1);
    EXPECT_FALSE(store()->load().valid);
}

TEST_F(EntitlementControllerTest, GatingOffBuildIsProEvenOnTheStore) {
    FakeStoreGateway gateway;
    ProBuildConfig config = gatedConfig();
    config.policy.cutover = QDateTime();
    auto c = make(InstallKind::Store, config, &gateway);
    c->start();
    EXPECT_FALSE(c->gatingEnabled());
    EXPECT_TRUE(c->isPro());
    gateway.answer(storeAnswer(kNow.addDays(-100)));
    EXPECT_TRUE(c->isPro());
}

TEST_F(EntitlementControllerTest, WithoutAGatewayAStoreBuildStartsALocalTrial) {
    auto c = make(InstallKind::Store, gatedConfig(), nullptr);
    c->start();
    EXPECT_TRUE(c->isTrial());
    EXPECT_EQ(c->sourceId(), QStringLiteral("localTrial"));
    EXPECT_EQ(c->trialDaysLeft(), 15);
    EXPECT_FALSE(c->storeAvailable());
    EXPECT_FALSE(c->canPurchase());
    EXPECT_TRUE(store()->load().valid);
}

TEST_F(EntitlementControllerTest, UnsupportedGatewayBehavesLikeNoGateway) {
    FakeStoreGateway gateway;
    gateway.supportedFlag = false;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    EXPECT_EQ(gateway.refreshCalls, 0);
    EXPECT_TRUE(c->isTrial());
    EXPECT_FALSE(c->canPurchase());
}

// --- Store answers ---------------------------------------------------------

TEST_F(EntitlementControllerTest, StartAsksTheStoreAndMarksRefreshing) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    EXPECT_EQ(gateway.refreshCalls, 1);
    EXPECT_TRUE(c->refreshing());
    gateway.answer(storeAnswer(kNow.addDays(-1)));
    EXPECT_FALSE(c->refreshing());
}

TEST_F(EntitlementControllerTest, GrandfatheredUserIsProAndCached) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    QSignalSpy changed(c.get(), &EntitlementController::changed);
    gateway.answer(storeAnswer(QDateTime(QDate(2025, 3, 1), QTime(0, 0), QTimeZone::UTC)));
    EXPECT_TRUE(c->isPro());
    EXPECT_EQ(c->sourceId(), QStringLiteral("grandfathered"));
    EXPECT_FALSE(c->canPurchase());
    EXPECT_GE(changed.count(), 1);
    const mf::EntitlementCache cache = store()->load();
    EXPECT_TRUE(cache.valid);
    EXPECT_EQ(cache.tier, Tier::Pro);
}

TEST_F(EntitlementControllerTest, NewUserGetsAStoreAnchoredTrialAndSeesThePrice) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-5), false, QStringLiteral("₹200.00")));
    EXPECT_TRUE(c->isTrial());
    EXPECT_EQ(c->sourceId(), QStringLiteral("storeTrial"));
    EXPECT_EQ(c->trialDaysLeft(), 10);
    EXPECT_EQ(c->price(), QStringLiteral("₹200.00"));
    EXPECT_TRUE(c->canPurchase());
    EXPECT_TRUE(c->isUnlocked(QStringLiteral("visualizer")));
}

TEST_F(EntitlementControllerTest, ExpiredTrialLocksFeatures) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20)));
    EXPECT_FALSE(c->unlocked());
    EXPECT_EQ(c->tierId(), QStringLiteral("free"));
    EXPECT_EQ(c->sourceId(), QStringLiteral("expired"));
    EXPECT_EQ(c->trialDaysLeft(), 0);
    for(const QString& id : c->featureIds())
        EXPECT_FALSE(c->isUnlocked(id)) << id.toStdString();
    EXPECT_TRUE(c->canPurchase());
}

TEST_F(EntitlementControllerTest, AddOnOwnerIsPro) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20), true));
    EXPECT_TRUE(c->isPro());
    EXPECT_EQ(c->sourceId(), QStringLiteral("addOn"));
    EXPECT_FALSE(c->canPurchase());
}

TEST_F(EntitlementControllerTest, StoreSilenceKeepsTheCachedProState) {
    FakeStoreGateway gateway;
    {
        auto first = make(InstallKind::Store, gatedConfig(), &gateway);
        first->start();
        gateway.answer(storeAnswer(QDateTime(QDate(2025, 3, 1), QTime(0, 0), QTimeZone::UTC)));
        ASSERT_TRUE(first->isPro());
    }
    auto second = make(InstallKind::Store, gatedConfig(), &gateway);
    second->start();
    EXPECT_TRUE(second->isPro());
    EXPECT_EQ(second->sourceId(), QStringLiteral("cached"));
    StoreSnapshot silent;
    silent.error = QStringLiteral("networkError");
    gateway.answer(silent);
    EXPECT_TRUE(second->isPro());
}

TEST_F(EntitlementControllerTest, LicensesChangedTriggersARefresh) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    ASSERT_EQ(gateway.refreshCalls, 1);
    gateway.changeLicenses();
    EXPECT_EQ(gateway.refreshCalls, 2);
}

// --- trial clock -----------------------------------------------------------

TEST_F(EntitlementControllerTest, TrialFlipsToFreeWhenTheClockPassesTheEnd) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-14)));
    ASSERT_TRUE(c->isTrial());
    EXPECT_EQ(c->trialDaysLeft(), 1);

    QSignalSpy changed(c.get(), &EntitlementController::changed);
    m_now = kNow.addDays(2);
    c->reevaluate();
    EXPECT_FALSE(c->unlocked());
    EXPECT_EQ(c->sourceId(), QStringLiteral("expired"));
    EXPECT_EQ(changed.count(), 1);
}

TEST_F(EntitlementControllerTest, ReevaluateWithoutChangeStaysQuiet) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-3)));
    QSignalSpy changed(c.get(), &EntitlementController::changed);
    m_now = kNow.addSecs(60);
    c->reevaluate();
    EXPECT_EQ(changed.count(), 0);
}

TEST_F(EntitlementControllerTest, ClockRollbackDoesNotReviveATrial) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20)));
    ASSERT_FALSE(c->unlocked());
    auto later = make(InstallKind::Store, gatedConfig(), nullptr);
    m_now = kNow.addDays(-10);
    later->start();
    EXPECT_FALSE(later->unlocked());
    EXPECT_TRUE(later->state().clockRolledBack);
}

// --- purchase --------------------------------------------------------------

TEST_F(EntitlementControllerTest, PurchaseGoesThroughTheGatewayAndRefreshesOnSuccess) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20)));
    ASSERT_TRUE(c->canPurchase());

    QWindow owner;
    QSignalSpy finished(c.get(), &EntitlementController::purchaseFinished);
    c->purchase(&owner);
    EXPECT_TRUE(c->purchasing());
    EXPECT_FALSE(c->canPurchase());
    EXPECT_EQ(gateway.purchases, QStringList{kProId});
    EXPECT_EQ(gateway.window, &owner);

    const int refreshes = gateway.refreshCalls;
    gateway.finishPurchase(kProId, true, mf::purchase::kSucceeded);
    EXPECT_FALSE(c->purchasing());
    EXPECT_EQ(c->lastPurchaseStatus(), mf::purchase::kSucceeded);
    EXPECT_EQ(gateway.refreshCalls, refreshes + 1);
    ASSERT_EQ(finished.count(), 1);
    EXPECT_TRUE(finished.at(0).at(0).toBool());
    EXPECT_EQ(finished.at(0).at(1).toString(), mf::purchase::kSucceeded);
    EXPECT_EQ(finished.at(0).at(2).toString(), kProId);

    gateway.answer(storeAnswer(kNow.addDays(-20), true));
    EXPECT_TRUE(c->isPro());
}

TEST_F(EntitlementControllerTest, CancelledPurchaseDoesNotRefresh) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20)));
    c->purchase();
    const int refreshes = gateway.refreshCalls;
    gateway.finishPurchase(kProId, false, mf::purchase::kNotPurchased);
    EXPECT_EQ(gateway.refreshCalls, refreshes);
    EXPECT_EQ(c->lastPurchaseStatus(), mf::purchase::kNotPurchased);
    EXPECT_TRUE(c->canPurchase());
}

TEST_F(EntitlementControllerTest, SecondPurchaseWhileBusyIsRejected) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20)));
    QSignalSpy finished(c.get(), &EntitlementController::purchaseFinished);
    c->purchase();
    c->purchase();
    EXPECT_EQ(gateway.purchases.size(), 1);
    ASSERT_EQ(finished.count(), 1);
    EXPECT_EQ(finished.at(0).at(1).toString(), mf::purchase::kBusy);
}

TEST_F(EntitlementControllerTest, PurchaseWhenAlreadyProReportsAlreadyPurchased) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20), true));
    QSignalSpy finished(c.get(), &EntitlementController::purchaseFinished);
    c->purchase();
    EXPECT_TRUE(gateway.purchases.isEmpty());
    ASSERT_EQ(finished.count(), 1);
    EXPECT_TRUE(finished.at(0).at(0).toBool());
    EXPECT_EQ(finished.at(0).at(1).toString(), mf::purchase::kAlreadyPurchased);
}

TEST_F(EntitlementControllerTest, PurchaseWithoutAStoreReportsUnavailable) {
    auto c = make(InstallKind::Store, gatedConfig(), nullptr);
    c->start();
    QSignalSpy finished(c.get(), &EntitlementController::purchaseFinished);
    c->purchase();
    ASSERT_EQ(finished.count(), 1);
    EXPECT_FALSE(finished.at(0).at(0).toBool());
    EXPECT_EQ(finished.at(0).at(1).toString(), mf::purchase::kUnavailable);
}

TEST_F(EntitlementControllerTest, PurchaseWithoutAnAddOnIdReportsNotConfigured) {
    FakeStoreGateway gateway;
    ProBuildConfig config = gatedConfig();
    config.proAddOnStoreId.clear();
    auto c = make(InstallKind::Store, config, &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20)));
    EXPECT_FALSE(c->canPurchase());
    QSignalSpy finished(c.get(), &EntitlementController::purchaseFinished);
    c->purchase();
    ASSERT_EQ(finished.count(), 1);
    EXPECT_EQ(finished.at(0).at(1).toString(), mf::purchase::kNotConfigured);
    EXPECT_TRUE(gateway.purchases.isEmpty());
}

TEST_F(EntitlementControllerTest, TipPurchaseUsesTheGivenId) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20), true));
    ASSERT_EQ(c->tipAddOnStoreIds(), QStringList{QStringLiteral("9NTIP001")});
    c->purchaseAddOn(QStringLiteral("9NTIP001"));
    EXPECT_EQ(gateway.purchases, QStringList{QStringLiteral("9NTIP001")});
    QSignalSpy finished(c.get(), &EntitlementController::purchaseFinished);
    gateway.finishPurchase(QStringLiteral("9NTIP001"), true, mf::purchase::kSucceeded);
    ASSERT_EQ(finished.count(), 1);
    EXPECT_EQ(finished.at(0).at(2).toString(), QStringLiteral("9NTIP001"));
}

// --- features and override -------------------------------------------------

TEST_F(EntitlementControllerTest, UnknownFeatureIdIsUnlocked) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->start();
    gateway.answer(storeAnswer(kNow.addDays(-20)));
    ASSERT_FALSE(c->unlocked());
    EXPECT_TRUE(c->isUnlocked(QStringLiteral("notAFeature")));
    EXPECT_FALSE(c->isUnlocked(QStringLiteral("perAppMixer")));
}

TEST_F(EntitlementControllerTest, FeatureIdsMatchTheModel) {
    auto c = make(InstallKind::Portable, gatedConfig(), nullptr);
    EXPECT_EQ(c->featureIds(), mf::allFeatureIds());
}

TEST_F(EntitlementControllerTest, ParseOverride) {
    EXPECT_EQ(EntitlementController::parseOverride(QStringLiteral("pro")), Tier::Pro);
    EXPECT_EQ(EntitlementController::parseOverride(QStringLiteral(" TRIAL ")), Tier::Trial);
    EXPECT_EQ(EntitlementController::parseOverride(QStringLiteral("Free")), Tier::Free);
    EXPECT_FALSE(EntitlementController::parseOverride(QStringLiteral("ultra")).has_value());
    EXPECT_FALSE(EntitlementController::parseOverride(QString()).has_value());
    EXPECT_EQ(EntitlementController::overrideEnvironmentVariable(), QStringLiteral("MEDIAFLYOUTS_ENTITLEMENT"));
}

TEST_F(EntitlementControllerTest, OverrideForcesTheTierAndIsNotCached) {
    FakeStoreGateway gateway;
    auto c = make(InstallKind::Store, gatedConfig(), &gateway);
    c->setOverride(Tier::Free);
    c->start();
    gateway.answer(storeAnswer(QDateTime(QDate(2025, 3, 1), QTime(0, 0), QTimeZone::UTC)));
    EXPECT_FALSE(c->unlocked());
    EXPECT_EQ(c->sourceId(), QStringLiteral("override"));
    EXPECT_FALSE(store()->load().valid);

    c->setOverride(Tier::Trial);
    EXPECT_TRUE(c->isTrial());
    EXPECT_EQ(c->trialDaysLeft(), 15);

    c->setOverride(std::nullopt);
    EXPECT_TRUE(c->isPro());
    EXPECT_EQ(c->sourceId(), QStringLiteral("grandfathered"));
}

#include "EntitlementControllerTest.moc"
