#include <gtest/gtest.h>

#include <QString>

#include "UpdateChecker.h"

TEST(UpdateCheckerTest, DetectsNewerVersions) {
    EXPECT_TRUE(UpdateChecker::isNewerVersion("0.1.0", "0.2.0"));
    EXPECT_TRUE(UpdateChecker::isNewerVersion("0.1.0", "1.0.0"));
    EXPECT_TRUE(UpdateChecker::isNewerVersion("0.1.0", "0.1.1"));
    EXPECT_TRUE(UpdateChecker::isNewerVersion("0.1.0", "v0.1.1")); // tolerates a leading v
    EXPECT_TRUE(UpdateChecker::isNewerVersion("1.2.3", "1.2.10")); // numeric, not lexical
}

TEST(UpdateCheckerTest, RejectsSameOrOlderVersions) {
    EXPECT_FALSE(UpdateChecker::isNewerVersion("0.1.0", "0.1.0"));
    EXPECT_FALSE(UpdateChecker::isNewerVersion("1.0.0", "0.9.9"));
    EXPECT_FALSE(UpdateChecker::isNewerVersion("0.1.0", ""));
    EXPECT_FALSE(UpdateChecker::isNewerVersion("2.0.0", "2"));
}

TEST(UpdateCheckerTest, DefaultState) {
    UpdateChecker u;
    EXPECT_EQ(u.currentVersion(), QStringLiteral("0.1.0"));
    EXPECT_TRUE(u.latestVersion().isEmpty());
    EXPECT_FALSE(u.updateAvailable());
    EXPECT_FALSE(u.checking());
}
