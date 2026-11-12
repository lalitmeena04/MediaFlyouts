#include <gtest/gtest.h>

#include "SessionSelection.h"

using namespace mf;

TEST(SessionSelectionTest, NormalizesCountToAtLeastOne) {
    EXPECT_EQ(normalizedSessionCount(0), 1);
    EXPECT_EQ(normalizedSessionCount(-1), 1);
    EXPECT_EQ(normalizedSessionCount(2), 2);
}

TEST(SessionSelectionTest, FindsTheMatchingAppId) {
    const QStringList appIds{"App.One", "App.Two", "App.Three"};
    EXPECT_EQ(selectedSessionIndexByAppId(appIds, "App.One"), 0);
    EXPECT_EQ(selectedSessionIndexByAppId(appIds, "App.Two"), 1);
    EXPECT_EQ(selectedSessionIndexByAppId(appIds, "App.Three"), 2);
}

TEST(SessionSelectionTest, FallsBackToZeroWhenTheAppIdIsNotFound) {
    const QStringList appIds{"App.One", "App.Two"};
    EXPECT_EQ(selectedSessionIndexByAppId(appIds, "App.Missing"), 0);
}

TEST(SessionSelectionTest, FallsBackToZeroWhenNothingIsSelected) {
    const QStringList appIds{"App.One", "App.Two"};
    EXPECT_EQ(selectedSessionIndexByAppId(appIds, QString()), 0);
    EXPECT_EQ(selectedSessionIndexByAppId({}, QString()), 0);
}

TEST(SessionSelectionTest, SelectsTheNewCurrentSessionWhenItArrivesSecond) {
    const QStringList appIds{"Music.App", "Video.App"};
    EXPECT_EQ(selectedSessionIndexByAppId(appIds, "Video.App"), 1);
}

TEST(SessionSelectionTest, SweepTargetsOnlyPlayingSessionsFromSweptApps) {
    const QStringList appIds{"Browser.App", "Music.App", "Browser.App"};
    const QList<bool> playing{true, true, false};
    const QStringList swept{"Browser.App"};
    EXPECT_EQ(pauseSweepTargets(appIds, playing, swept), (QList<int>{0}));
}

TEST(SessionSelectionTest, SweepTargetsPromotedBrowserTabSession) {
    // Browser paused tab one, then promoted tab two as new playing session.
    const QStringList appIds{"Browser.App"};
    const QList<bool> playing{true};
    const QStringList swept{"Browser.App", "Music.App"};
    EXPECT_EQ(pauseSweepTargets(appIds, playing, swept), (QList<int>{0}));
}

TEST(SessionSelectionTest, SweepIgnoresAppsThatWereNotPaused) {
    const QStringList appIds{"Other.App", "Music.App"};
    const QList<bool> playing{true, true};
    const QStringList swept{"Browser.App"};
    EXPECT_TRUE(pauseSweepTargets(appIds, playing, swept).isEmpty());
}

TEST(SessionSelectionTest, SweepIgnoresEmptyAppIds) {
    const QStringList appIds{QString(), "Browser.App"};
    const QList<bool> playing{true, true};
    const QStringList swept{QString(), "Browser.App"};
    EXPECT_EQ(pauseSweepTargets(appIds, playing, swept), (QList<int>{1}));
}

TEST(SessionSelectionTest, SweepHandlesMismatchedListSizes) {
    const QStringList appIds{"Browser.App", "Browser.App"};
    const QList<bool> playing{true};
    const QStringList swept{"Browser.App"};
    EXPECT_EQ(pauseSweepTargets(appIds, playing, swept), (QList<int>{0}));
    EXPECT_TRUE(pauseSweepTargets({}, {}, swept).isEmpty());
}
