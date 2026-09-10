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
    EXPECT_EQ(u.currentVersion(), QStringLiteral(MEDIAFLYOUTS_VERSION));
    EXPECT_TRUE(u.latestVersion().isEmpty());
    EXPECT_FALSE(u.updateAvailable());
    EXPECT_FALSE(u.checking());
}

TEST(UpdateCheckerTest, UsesReleaseRepository) {
    EXPECT_EQ(UpdateChecker::releasesApiUrl(),
              QStringLiteral("https://api.github.com/repos/scleaverzer0ne/MediaFlyouts-Releases/releases/latest"));
    EXPECT_EQ(UpdateChecker::releasesPageUrl(),
              QStringLiteral("https://github.com/scleaverzer0ne/MediaFlyouts-Releases/releases/latest"));
}

// Without a check the download button still reaches the releases index.
TEST(UpdateCheckerTest, ReleaseUrlFallsBackToTheReleasesPage) {
    UpdateChecker u;
    EXPECT_EQ(u.releaseUrl(), UpdateChecker::releasesPageUrl());
}

TEST(UpdateCheckerTest, TrustsOnlyHttpsGitHubLinks) {
    EXPECT_TRUE(UpdateChecker::isTrustedReleaseUrl(
        QStringLiteral("https://github.com/scleaverzer0ne/MediaFlyouts-Releases/releases/tag/v1.3.0")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QStringLiteral("http://github.com/owner/repo")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QStringLiteral("https://github.com.evil.test/owner/repo")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QStringLiteral("file:///C:/Windows/System32/cmd.exe")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QString()));
}
