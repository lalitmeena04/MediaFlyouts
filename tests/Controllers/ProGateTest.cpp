#include <gtest/gtest.h>

#include <memory>

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimeZone>

#include "EntitlementController.h"
#include "EntitlementStore.h"
#include "ProGate.h"
#include "Settings.h"
#include "TestSupport/IsolatedSettings.h"

using mf::InstallKind;
using mf::ProFeature;
using mf::Tier;

namespace {

class ProGateTest : public ::testing::Test {
    protected:
        void SetUp() override {
            mftest::clearUserSettings();
            ASSERT_TRUE(m_dir.isValid());
            mf::ProBuildConfig config;
            config.proAddOnStoreId = QStringLiteral("9NPRO");
            config.policy.cutover = QDateTime(QDate(2026, 12, 1), QTime(0, 0), QTimeZone::UTC);
            m_settings = std::make_unique<Settings>();
            m_entitlement = std::make_unique<EntitlementController>(
                InstallKind::Store, config,
                std::make_unique<mf::EntitlementStore>(m_dir.filePath(QStringLiteral("e.dat")),
                                                       std::make_unique<mf::ChecksumCodec>()));
            m_entitlement->start();
            m_gate = std::make_unique<ProGate>(m_settings.get(), m_entitlement.get());
        }

        void lock() { m_entitlement->setOverride(Tier::Free); }
        void unlock() { m_entitlement->setOverride(Tier::Pro); }

        // Every gated setting set to the opposite of its free-tier default.
        void setNonDefaults() {
            m_settings->setTaskbarVisualizerEnabled(true);
            m_settings->setVolumeFlyoutMixerEnabled(true);
            m_settings->setPeripheralsWidgetEnabled(true);
            m_settings->setPeripheralFlyoutEnabled(true);
            m_settings->setPeripheralLowBatteryToast(true);
            m_settings->setNextUpStyle(1);
            m_settings->setTaskbarWidgetControlsEnabled(false);
            m_settings->setTaskbarWidgetPauseAllEnabled(true);
            m_settings->setTaskbarWidgetPlayAllEnabled(false);
            m_settings->setTaskbarWidgetLayout(1);
            m_settings->setTaskbarWidgetShowPauseOverlay(false);
            m_settings->setTaskbarWidgetScrollingEnabled(false);
            m_settings->setTaskbarWidgetAdaptiveColors(false);
        }

        QTemporaryDir m_dir;
        std::unique_ptr<Settings> m_settings;
        std::unique_ptr<EntitlementController> m_entitlement;
        std::unique_ptr<ProGate> m_gate;
};

} // namespace

TEST_F(ProGateTest, TrialStartsUnlocked) {
    ASSERT_TRUE(m_entitlement->isTrial());
    for(ProFeature f : {ProFeature::Visualizer, ProFeature::PerAppMixer, ProFeature::PeripheralBattery,
                        ProFeature::NowPlayingStyles, ProFeature::TaskbarWidgetOptions})
        EXPECT_TRUE(m_gate->unlocked(f));
}

TEST_F(ProGateTest, UnlockedPassesTheStoredSettingsThrough) {
    unlock();
    setNonDefaults();
    EXPECT_TRUE(m_gate->visualizerEnabled());
    EXPECT_TRUE(m_gate->mixerEnabled());
    EXPECT_TRUE(m_gate->mixerWindowAllowed());
    EXPECT_TRUE(m_gate->peripheralsWidgetEnabled());
    EXPECT_TRUE(m_gate->peripheralFlyoutEnabled());
    EXPECT_TRUE(m_gate->peripheralLowBatteryToast());
    EXPECT_EQ(m_gate->nextUpStyle(), 1);
    EXPECT_FALSE(m_gate->taskbarWidgetControlsEnabled());
    EXPECT_TRUE(m_gate->taskbarWidgetPauseAllEnabled());
    EXPECT_FALSE(m_gate->taskbarWidgetPlayAllEnabled());
    EXPECT_EQ(m_gate->taskbarWidgetLayout(), 1);
    EXPECT_FALSE(m_gate->taskbarWidgetShowPauseOverlay());
    EXPECT_FALSE(m_gate->taskbarWidgetScrollingEnabled());
    EXPECT_FALSE(m_gate->taskbarWidgetAdaptiveColors());
}

TEST_F(ProGateTest, LockedForcesTheFreeTierDefaults) {
    setNonDefaults();
    lock();
    EXPECT_FALSE(m_gate->visualizerEnabled());
    EXPECT_FALSE(m_gate->mixerEnabled());
    EXPECT_FALSE(m_gate->mixerWindowAllowed());
    EXPECT_FALSE(m_gate->peripheralsWidgetEnabled());
    EXPECT_FALSE(m_gate->peripheralFlyoutEnabled());
    EXPECT_FALSE(m_gate->peripheralLowBatteryToast());
    EXPECT_EQ(m_gate->nextUpStyle(), 0);
    EXPECT_TRUE(m_gate->taskbarWidgetControlsEnabled());
    EXPECT_FALSE(m_gate->taskbarWidgetPauseAllEnabled());
    EXPECT_TRUE(m_gate->taskbarWidgetPlayAllEnabled());
    EXPECT_EQ(m_gate->taskbarWidgetLayout(), 0);
    EXPECT_TRUE(m_gate->taskbarWidgetShowPauseOverlay());
    EXPECT_TRUE(m_gate->taskbarWidgetScrollingEnabled());
    EXPECT_TRUE(m_gate->taskbarWidgetAdaptiveColors());
}

TEST_F(ProGateTest, LockingKeepsTheStoredSettingsIntact) {
    setNonDefaults();
    lock();
    EXPECT_TRUE(m_settings->taskbarVisualizerEnabled());
    EXPECT_EQ(m_settings->nextUpStyle(), 1);
    unlock();
    EXPECT_TRUE(m_gate->visualizerEnabled());
    EXPECT_EQ(m_gate->nextUpStyle(), 1);
}

TEST_F(ProGateTest, ChangedFollowsSettingsAndEntitlement) {
    QSignalSpy changed(m_gate.get(), &ProGate::changed);
    m_settings->setTaskbarVisualizerEnabled(true);
    EXPECT_EQ(changed.count(), 1);
    lock();
    EXPECT_GE(changed.count(), 2);
}

TEST_F(ProGateTest, WithoutAControllerNothingIsGated) {
    ProGate open(m_settings.get(), nullptr);
    setNonDefaults();
    EXPECT_TRUE(open.visualizerEnabled());
    EXPECT_EQ(open.nextUpStyle(), 1);
    EXPECT_TRUE(open.unlocked(ProFeature::PerAppMixer));
}

TEST_F(ProGateTest, WithoutSettingsEverythingReadsAsDefault) {
    unlock();
    ProGate bare(nullptr, m_entitlement.get());
    EXPECT_FALSE(bare.visualizerEnabled());
    EXPECT_EQ(bare.nextUpStyle(), 0);
    EXPECT_TRUE(bare.taskbarWidgetControlsEnabled());
}
