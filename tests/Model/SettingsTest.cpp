#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUrl>

#include "Settings.h"

namespace {
// Redirect QSettings to a throwaway location and start each case from a clean slate.
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
    EXPECT_FALSE(s.repeatEnabled());
    EXPECT_FALSE(s.shuffleEnabled());
    EXPECT_TRUE(s.volumeFlyoutEnabled());
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
    EXPECT_FALSE(s.launchOnStartup());
    EXPECT_FALSE(s.trayIconHidden());
    EXPECT_FALSE(s.trayLeftClickShowsFlyout());
    EXPECT_FALSE(s.taskbarWidgetEnabled());
    EXPECT_TRUE(s.taskbarWidgetControlsEnabled());
    EXPECT_EQ(s.flyoutPosition(), 4);
    EXPECT_EQ(s.flyoutMonitor(), 0);
    EXPECT_EQ(s.flyoutDuration(), 4500);
    EXPECT_EQ(s.flyoutAnimationSpeed(), 2);
    EXPECT_EQ(s.flyoutAnimationMs(), 300);
    EXPECT_EQ(s.flyoutAnimationEasingStyle(), 3);
    EXPECT_EQ(s.volumeFlyoutDuration(), 1800);
    EXPECT_EQ(s.nextUpDuration(), 2000);
    EXPECT_EQ(s.nextUpLeadTime(), 3000);
    EXPECT_EQ(s.taskbarWidgetOffset(), 0);
    EXPECT_EQ(s.taskbarWidgetSide(), 2);
    EXPECT_EQ(s.taskbarWidgetMonitor(), 0);
    EXPECT_TRUE(s.taskbarWidgetShowPauseOverlay());
    EXPECT_FALSE(s.taskbarWidgetScrollingEnabled());
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
    s.setMediaFlyoutEnabled(true); // already the default
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(SettingsTest, EachToggleIsIndependent) {
    Settings s;
    s.setVolumeFlyoutEnabled(false);
    s.setLaunchOnStartup(true);
    EXPECT_FALSE(s.volumeFlyoutEnabled());
    EXPECT_TRUE(s.launchOnStartup());
    // untouched toggles keep their defaults
    EXPECT_TRUE(s.mediaFlyoutEnabled());
    EXPECT_TRUE(s.overrideNativeOsd());
}

TEST_F(SettingsTest, ValuesPersistAcrossInstances) {
    {
        Settings first;
        first.setOverrideNativeOsd(false);
        first.setLaunchOnStartup(true);
    }
    Settings second;
    EXPECT_FALSE(second.overrideNativeOsd());
    EXPECT_TRUE(second.launchOnStartup());
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
    // wipe the live store so defaults come back, then restore from the export
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
