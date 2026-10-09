#include <gtest/gtest.h>

#include <QString>

#include "InstallSource.h"

using mf::InstallKind;

TEST(InstallSourceTest, PackagedProcessIsAStoreInstall) {
    // Package identity wins even when an installer entry exists.
    EXPECT_EQ(mf::classifyInstall(true, QStringLiteral("C:/Program Files/MediaFlyouts"),
                                  QStringLiteral("C:/Program Files/MediaFlyouts")),
              InstallKind::Store);
    EXPECT_EQ(mf::classifyInstall(true, QString(), QStringLiteral("C:/Apps/MediaFlyouts")), InstallKind::Store);
}

TEST(InstallSourceTest, NoUninstallEntryMeansPortable) {
    EXPECT_EQ(mf::classifyInstall(false, QString(), QStringLiteral("D:/Tools/MediaFlyouts")), InstallKind::Portable);
}

TEST(InstallSourceTest, RunningFromTheInstalledFolderIsAnInstallerInstall) {
    // Inno writes a native path with a trailing backslash. The app reports forward slashes.
    EXPECT_EQ(mf::classifyInstall(false, QStringLiteral("C:\\Program Files\\MediaFlyouts\\"),
                                  QStringLiteral("C:/Program Files/MediaFlyouts")),
              InstallKind::Installer);
    // Case differences on Windows paths must not matter.
    EXPECT_EQ(mf::classifyInstall(false, QStringLiteral("c:\\program files\\mediaflyouts"),
                                  QStringLiteral("C:/Program Files/MediaFlyouts/")),
              InstallKind::Installer);
}

// A ZIP copy next to an installed copy must not run the setup over itself.
TEST(InstallSourceTest, RunningElsewhereWithAnInstallEntryIsPortable) {
    EXPECT_EQ(mf::classifyInstall(false, QStringLiteral("C:\\Program Files\\MediaFlyouts\\"),
                                  QStringLiteral("D:/Downloads/MediaFlyouts-1.4.1-win64")),
              InstallKind::Portable);
}

TEST(InstallSourceTest, IdsAndLabelsAreStable) {
    EXPECT_EQ(mf::installKindId(InstallKind::Store), QStringLiteral("store"));
    EXPECT_EQ(mf::installKindId(InstallKind::Installer), QStringLiteral("installer"));
    EXPECT_EQ(mf::installKindId(InstallKind::Portable), QStringLiteral("portable"));
    EXPECT_EQ(mf::installKindId(InstallKind::Unknown), QStringLiteral("unknown"));
    EXPECT_EQ(mf::installKindLabel(InstallKind::Store), QStringLiteral("Microsoft Store"));
    EXPECT_EQ(mf::installKindLabel(InstallKind::Installer), QStringLiteral("Installer"));
    EXPECT_EQ(mf::installKindLabel(InstallKind::Portable), QStringLiteral("Portable"));
    EXPECT_EQ(mf::installKindLabel(InstallKind::Unknown), QStringLiteral("Unknown"));
}

TEST(InstallSourceTest, StoreUrlUsesTheStoreScheme) {
    EXPECT_TRUE(mf::storeUpdatesUrl().startsWith(QStringLiteral("ms-windows-store://")));
}

// The test binary is neither packaged nor installed by Inno Setup.
TEST(InstallSourceTest, DetectionRunsWithoutCrashing) {
    const InstallKind kind = mf::detectInstallKind();
    EXPECT_TRUE(kind == InstallKind::Portable || kind == InstallKind::Unknown);
}
