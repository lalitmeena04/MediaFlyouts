#include <gtest/gtest.h>

#include <QColor>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>

#include "Settings.h"
#include "NowPlaying.h"
#include "ThemeController.h"

namespace {
// Redirect QSettings to a throwaway location so appTheme changes don't touch real settings.
class ThemeControllerTest : public ::testing::Test {
    protected:
        void SetUp() override {
            QStandardPaths::setTestModeEnabled(true);
            QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                      QStringLiteral("MediaFlyouts"))
                .clear();
        }
};
} // namespace

TEST_F(ThemeControllerTest, DarkPalette) {
    Settings s;
    s.setAppTheme(2); // force dark
    ThemeController t(&s);
    EXPECT_TRUE(t.dark());
    EXPECT_EQ(t.window(), QColor("#1B1B1B"));
    EXPECT_EQ(t.accent(), QColor("#FFFFFF"));
    EXPECT_EQ(t.textPrimary(), QColor("#FFFFFF"));
    EXPECT_EQ(t.card(), QColor("#232323"));
}

TEST_F(ThemeControllerTest, LightPalette) {
    Settings s;
    s.setAppTheme(1); // force light
    ThemeController t(&s);
    EXPECT_FALSE(t.dark());
    EXPECT_EQ(t.window(), QColor("#F3F3F3"));
    EXPECT_EQ(t.accent(), QColor("#000000"));
    EXPECT_EQ(t.textPrimary(), QColor("#1A1A1A"));
    EXPECT_EQ(t.card(), QColor("#FFFFFF"));
}

TEST_F(ThemeControllerTest, EmitsChangedWhenModeFlips) {
    Settings s;
    s.setAppTheme(1); // light
    ThemeController t(&s);
    ASSERT_FALSE(t.dark());

    QSignalSpy spy(&t, &ThemeController::changed);
    s.setAppTheme(2); // -> dark
    EXPECT_TRUE(t.dark());
    EXPECT_GE(spy.count(), 1);
}

TEST_F(ThemeControllerTest, CustomAccentOverridesDefault) {
    Settings s;
    s.setAppTheme(2); // dark default accent
    ThemeController t(&s);
    EXPECT_EQ(t.accent(), QColor("#FFFFFF"));
    s.setAccentMode(2);
    s.setCustomAccent(QStringLiteral("#FF8800"));
    EXPECT_EQ(t.accent(), QColor("#FF8800"));
}

TEST_F(ThemeControllerTest, SystemAccentAppliesWhenMatchingSystem) {
    Settings s;
    s.setAccentMode(1); // match system
    ThemeController t(&s);
    t.setSystemAccent(QColor("#123456"));
    EXPECT_EQ(t.accent(), QColor("#123456"));
}

TEST_F(ThemeControllerTest, AlbumArtAccentOverridesThemeAccentWhenEnabled) {
    Settings s;
    NowPlaying nowPlaying;
    ThemeController t(&s, &nowPlaying);
    s.setAccentFromArt(true);

    QImage art(8, 8, QImage::Format_ARGB32);
    art.fill(Qt::red);
    nowPlaying.applyArt(art);

    EXPECT_EQ(t.accent(), nowPlaying.artAccent());
}
