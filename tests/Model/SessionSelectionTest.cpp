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

// Regression test. WinRT identity differs across SMTC calls.
// App id matching fixes stuck indicator state.
TEST(SessionSelectionTest, SelectsTheNewCurrentSessionWhenItArrivesSecond) {
    const QStringList appIds{"Music.App", "Video.App"};
    EXPECT_EQ(selectedSessionIndexByAppId(appIds, "Video.App"), 1);
}
