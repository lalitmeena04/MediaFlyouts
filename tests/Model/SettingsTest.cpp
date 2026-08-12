#include <gtest/gtest.h>

#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>

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
    EXPECT_TRUE(s.volumeFlyoutEnabled());
    EXPECT_TRUE(s.overrideNativeOsd());
    EXPECT_TRUE(s.lockKeysEnabled());
    EXPECT_TRUE(s.nextUpEnabled());
    EXPECT_TRUE(s.disableOnFullscreen());
    EXPECT_FALSE(s.launchOnStartup());
    EXPECT_FALSE(s.taskbarWidgetEnabled());
    EXPECT_TRUE(s.taskbarWidgetControlsEnabled());
    EXPECT_EQ(s.flyoutPosition(), 4);
    EXPECT_EQ(s.flyoutMonitor(), 0);
    EXPECT_EQ(s.flyoutDuration(), 4500);
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
