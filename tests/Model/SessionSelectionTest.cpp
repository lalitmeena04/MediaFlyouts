#include <gtest/gtest.h>

#include "SessionSelection.h"

using namespace mf;

TEST(SessionSelectionTest, NormalizesAnEmptySessionListToOne) {
    EXPECT_EQ(normalizedSessionCount(0), 1);
    EXPECT_EQ(normalizedSessionCount(-1), 1);
    EXPECT_EQ(normalizedSessionCount(2), 2);
}

TEST(SessionSelectionTest, SelectsTheFirstMatchingSession) {
    EXPECT_EQ(selectedSessionIndex({true}), 0);
    EXPECT_EQ(selectedSessionIndex({false, true}), 1);
    EXPECT_EQ(selectedSessionIndex({false, false, true}), 2);
}

TEST(SessionSelectionTest, SelectsTheNewCurrentSessionWhenItArrivesSecond) {
    // Regression: the widget showed the second app's track while its dot remained at index 0.
    EXPECT_EQ(selectedSessionIndex({false, true}), 1);
}

TEST(SessionSelectionTest, FallsBackToTheFirstDotWhenTheCurrentSessionVanished) {
    EXPECT_EQ(selectedSessionIndex({}), 0);
    EXPECT_EQ(selectedSessionIndex({false, false}), 0);
}
