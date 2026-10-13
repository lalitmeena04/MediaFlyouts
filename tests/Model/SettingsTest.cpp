#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUrl>

#include "Settings.h"

namespace {
class SettingsTest : public ::testing::Test {
    protected:
        void SetUp() override {
            QStandardPaths::setTestModeEnabled(true);
            QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                      QStringLiteral("MediaFlyouts"))
                .clear();
        }
};
} // namespace

TEST_F(SettingsTest, Defaults) {
    Settings s;
    EXPECT_TRUE(s.mediaFlyoutEnabled());
    EXPECT_TRUE(s.seekbarEnabled());
    EXPECT_TRUE(s.seekbarInteractive());
    EXPECT_FALSE(s.mediaFlyoutAlwaysDisplay());
    EXPECT_FALSE(s.centerTitleArtist());
    EXPECT_FALSE(s.compactLayout());
    EXPECT_TRUE(s.playerInfoEnabled());
    EXPECT_TRUE(s.repeatEnabled());
    EXPECT_TRUE(s.shuffleEnabled());
    EXPECT_TRUE(s.volumeFlyoutEnabled());
    EXPECT_FALSE(s.volumeFlyoutMixerEnabled());
    EXPECT_TRUE(s.overrideNativeOsd());
    EXPECT_TRUE(s.lockKeysEnabled());
    EXPECT_TRUE(s.lockKeysCapsEnabled());
    EXPECT_TRUE(s.lockKeysNumEnabled());
    EXPECT_TRUE(s.lockKeysScrollEnabled());
    EXPECT_FALSE(s.lockKeysInsertEnabled());
    EXPECT_TRUE(s.lockKeysBoldUi());
    EXPECT_EQ(s.lockKeysDuration(), 2000);
    EXPECT_TRUE(s.nextUpEnabled());
    EXPECT_TRUE(s.disableOnFullscreen());
    EXPECT_TRUE(s.flyoutSwipeDismissEnabled());
    EXPECT_TRUE(s.launchOnStartup());
    EXPECT_FALSE(s.trayIconHidden());
    EXPECT_TRUE(s.trayLeftClickShowsFlyout());
    EXPECT_TRUE(s.taskbarWidgetEnabled());
    EXPECT_TRUE(s.taskbarWidgetControlsEnabled());
    EXPECT_EQ(s.flyoutPosition(), 4);
    EXPECT_EQ(s.flyoutMonitor(), 0);
    EXPECT_EQ(s.flyoutDuration(), 4500);
    EXPECT_EQ(s.flyoutAnimationSpeed(), 2);
    EXPECT_EQ(s.flyoutAnimationMs(), 300);
    EXPECT_EQ(s.flyoutAnimationEasingStyle(), 3);
    EXPECT_EQ(s.volumeFlyoutDuration(), 1800);
    EXPECT_EQ(s.volumeFlyoutPosition(), 4);
    EXPECT_EQ(s.volumeFlyoutMonitor(), 0);
    EXPECT_EQ(s.nextUpDuration(), 2000);
    EXPECT_EQ(s.nextUpStyle(), 0);
    EXPECT_EQ(s.taskbarWidgetOffset(), 0);
    EXPECT_TRUE(s.taskbarWidgetSide() == 0 || s.taskbarWidgetSide() == 2);
    EXPECT_EQ(s.taskbarWidgetMonitor(), 0);
    EXPECT_EQ(s.taskbarWidgetLayout(), 0);
    EXPECT_TRUE(s.taskbarWidgetShowPauseOverlay());
    EXPECT_TRUE(s.taskbarWidgetScrollingEnabled());
    EXPECT_FALSE(s.taskbarWidgetPauseAllEnabled());
    EXPECT_TRUE(s.taskbarWidgetPlayAllEnabled());
    EXPECT_FALSE(s.taskbarVisualizerEnabled());
    EXPECT_TRUE(s.taskbarVisualizerCentered());
    EXPECT_EQ(s.visualizerStyle(), 0);
    EXPECT_FALSE(s.onboardingCompleted());
    EXPECT_FALSE(s.accentFromArt());
    EXPECT_TRUE(s.accentFromArtAppWide());
    EXPECT_EQ(s.accentMode(), 0);
    EXPECT_EQ(s.customAccent(), QStringLiteral("#4CC2FF"));
    EXPECT_EQ(s.appTheme(), 0);
    EXPECT_EQ(s.filterMode(), 0);
    EXPECT_TRUE(s.filterList().isEmpty());
    EXPECT_FALSE(s.autoCheckUpdates());
    EXPECT_TRUE(s.skippedUpdateVersion().isEmpty());
    EXPECT_FALSE(s.peripheralFlyoutEnabled());
    EXPECT_EQ(s.peripheralFlyoutDuration(), 3000);
    EXPECT_EQ(s.peripheralFlyoutPosition(), 4);
    EXPECT_EQ(s.peripheralFlyoutMonitor(), 0);
    EXPECT_TRUE(s.peripheralFlyoutOnConnect());
    EXPECT_TRUE(s.peripheralFlyoutOnDisconnect());
    EXPECT_TRUE(s.peripheralFlyoutOnLowBattery());
    EXPECT_FALSE(s.peripheralFlyoutOnCharging());
    EXPECT_TRUE(s.peripheralLowBatteryToast());
    EXPECT_FALSE(s.peripheralFlyoutOnWired());
    EXPECT_FALSE(s.peripheralsWidgetWiredEnabled());
    EXPECT_FALSE(s.chargerFlyoutEnabled());
    EXPECT_TRUE(s.chargerFlyoutOnPlug());
    EXPECT_TRUE(s.chargerFlyoutOnUnplug());
    EXPECT_EQ(s.chargerFlyoutDuration(), 3000);
    EXPECT_EQ(s.chargerFlyoutPosition(), 4);
    EXPECT_EQ(s.chargerFlyoutMonitor(), 0);
}

TEST_F(SettingsTest, SkippedUpdateVersionIsStoredAndRestorable) {
    {
        Settings s;
        QSignalSpy spy(&s, &Settings::changed);
        s.setSkippedUpdateVersion(QStringLiteral("v1.5.0"));
        s.setSkippedUpdateVersion(QStringLiteral("v1.5.0"));
        EXPECT_EQ(spy.count(), 1);
        EXPECT_EQ(s.skippedUpdateVersion(), QStringLiteral("v1.5.0"));
    }
    Settings reloaded;
    EXPECT_EQ(reloaded.skippedUpdateVersion(), QStringLiteral("v1.5.0"));
}

TEST_F(SettingsTest, PeripheralNameFallsBackToTheDetectedName) {
    Settings s;
    EXPECT_EQ(s.peripheralDisplayName(QStringLiteral("2.4G Wireless Receiver")),
              QStringLiteral("2.4G Wireless Receiver"));
    EXPECT_TRUE(s.peripheralNames().isEmpty());
}

TEST_F(SettingsTest, PeripheralRenameIsStoredAndRestorable) {
    Settings s;
    QSignalSpy spy(&s, &Settings::changed);

    s.setPeripheralDisplayName(QStringLiteral("2.4G Wireless Receiver"), QStringLiteral("AULA F75"));
    EXPECT_EQ(s.peripheralDisplayName(QStringLiteral("2.4G Wireless Receiver")), QStringLiteral("AULA F75"));
    EXPECT_EQ(spy.count(), 1);

    s.setPeripheralDisplayName(QStringLiteral("2.4G Wireless Receiver"), QStringLiteral("AULA F75"));
    EXPECT_EQ(spy.count(), 1);

    s.setPeripheralDisplayName(QStringLiteral("2.4G Wireless Receiver"), QString());
    EXPECT_EQ(s.peripheralDisplayName(QStringLiteral("2.4G Wireless Receiver")),
              QStringLiteral("2.4G Wireless Receiver"));
    EXPECT_TRUE(s.peripheralNames().isEmpty());
    EXPECT_EQ(spy.count(), 2);
}

TEST_F(SettingsTest, PeripheralRenameMatchingTheDetectedNameStoresNothing) {
    Settings s;
    s.setPeripheralDisplayName(QStringLiteral("Gaming Keyboard"), QStringLiteral("  Gaming Keyboard  "));
    EXPECT_TRUE(s.peripheralNames().isEmpty());
}

TEST_F(SettingsTest, PeripheralRenameSurvivesReloadAndAwkwardNames) {
    const QString awkward = QStringLiteral("Owl Lite / Audio = [main]");
    {
        Settings first;
        first.setPeripheralDisplayName(awkward, QStringLiteral("Webcam"));
    }
    Settings second;
    EXPECT_EQ(second.peripheralDisplayName(awkward), QStringLiteral("Webcam"));
}

TEST_F(SettingsTest, PeripheralRenameIgnoresEmptyDeviceAndCapsLength) {
    Settings s;
    s.setPeripheralDisplayName(QString(), QStringLiteral("nothing"));
    EXPECT_TRUE(s.peripheralNames().isEmpty());

    s.setPeripheralDisplayName(QStringLiteral("Mouse"), QString(200, QLatin1Char('x')));
    EXPECT_EQ(s.peripheralDisplayName(QStringLiteral("Mouse")).size(), 64);
}

TEST_F(SettingsTest, SetterUpdatesValueAndEmitsChanged) {
    Settings s;
    QSignalSpy spy(&s, &Settings::changed);
    s.setMediaFlyoutEnabled(false);
    EXPECT_FALSE(s.mediaFlyoutEnabled());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(SettingsTest, SettingSameValueDoesNotEmit) {
    Settings s;
    QSignalSpy spy(&s, &Settings::changed);
    s.setMediaFlyoutEnabled(true);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(SettingsTest, EachToggleIsIndependent) {
    Settings s;
    s.setVolumeFlyoutEnabled(false);
    s.setLaunchOnStartup(true);
    EXPECT_FALSE(s.volumeFlyoutEnabled());
    EXPECT_TRUE(s.launchOnStartup());
    EXPECT_TRUE(s.mediaFlyoutEnabled());
    EXPECT_TRUE(s.overrideNativeOsd());
}

TEST_F(SettingsTest, ValuesPersistAcrossInstances) {
    {
        Settings first;
        first.setOverrideNativeOsd(false);
        first.setLaunchOnStartup(true);
        first.setFlyoutSwipeDismissEnabled(false);
    }
    Settings second;
    EXPECT_FALSE(second.overrideNativeOsd());
    EXPECT_TRUE(second.launchOnStartup());
    EXPECT_FALSE(second.flyoutSwipeDismissEnabled());
}

TEST_F(SettingsTest, TaskbarWidgetSideIsSeededOnceThenPinned) {
    int seeded = -1;
    {
        Settings first;
        seeded = first.taskbarWidgetSide();
        first.setTaskbarWidgetSide(seeded == 1 ? 0 : 1);
    }
    Settings second;
    EXPECT_EQ(second.taskbarWidgetSide(), seeded == 1 ? 0 : 1);
}

TEST_F(SettingsTest, ExportImportRoundTrip) {
    const QString path = QDir::temp().filePath(QStringLiteral("mf_settings_export.ini"));
    QFile::remove(path);
    {
        Settings s;
        s.setMediaFlyoutEnabled(false);
        s.setFlyoutDuration(7000);
        EXPECT_TRUE(s.exportTo(QUrl::fromLocalFile(path)));
    }
    QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
              QStringLiteral("MediaFlyouts"))
        .clear();
    Settings restored;
    EXPECT_TRUE(restored.mediaFlyoutEnabled());
    EXPECT_TRUE(restored.importFrom(QUrl::fromLocalFile(path)));
    EXPECT_FALSE(restored.mediaFlyoutEnabled());
    EXPECT_EQ(restored.flyoutDuration(), 7000);
    QFile::remove(path);
}

TEST_F(SettingsTest, FilterOffAllowsEveryPlayer) {
    Settings s;
    s.addFilterEntry(QStringLiteral("Spotify"));
    EXPECT_EQ(s.filterMode(), 0);
    EXPECT_TRUE(s.playerAllowed(QStringLiteral("Spotify")));
    EXPECT_TRUE(s.playerAllowed(QStringLiteral("chrome")));
}

TEST_F(SettingsTest, BlacklistHidesListedPlayers) {
    Settings s;
    s.setFilterMode(1);
    s.addFilterEntry(QStringLiteral("chrome"));
    EXPECT_FALSE(s.playerAllowed(QStringLiteral("chrome")));
    EXPECT_FALSE(s.playerAllowed(QStringLiteral("Chrome")));
    EXPECT_TRUE(s.playerAllowed(QStringLiteral("Spotify")));
}

TEST_F(SettingsTest, WhitelistShowsOnlyListedPlayers) {
    Settings s;
    s.setFilterMode(2);
    s.addFilterEntry(QStringLiteral("Spotify"));
    EXPECT_TRUE(s.playerAllowed(QStringLiteral("Spotify")));
    EXPECT_FALSE(s.playerAllowed(QStringLiteral("chrome")));
}

TEST_F(SettingsTest, FilterEntriesAreUniqueAndRemovable) {
    Settings s;
    s.addFilterEntry(QStringLiteral("Spotify"));
    s.addFilterEntry(QStringLiteral("spotify"));
    s.addFilterEntry(QStringLiteral("  "));
    EXPECT_EQ(s.filterList().size(), 1);

    s.removeFilterEntry(QStringLiteral("SPOTIFY"));
    EXPECT_TRUE(s.filterList().isEmpty());
}
