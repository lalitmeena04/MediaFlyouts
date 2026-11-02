#include <gtest/gtest.h>

#include <QString>

#include "InstallSource.h"

using mf::InstallKind;

TEST(InstallSourceTest, PackagedProcessIsAStoreInstall) {
    EXPECT_EQ(mf::classifyInstall(true, QStringLiteral("C:/Program Files/MediaFlyouts"),
                                  QStringLiteral("C:/Program Files/MediaFlyouts")),
              InstallKind::Store);
    EXPECT_EQ(mf::classifyInstall(true, QString(), QStringLiteral("C:/Apps/MediaFlyouts")), InstallKind::Store);
}

TEST(InstallSourceTest, NoUninstallEntryMeansPortable) {
    EXPECT_EQ(mf::classifyInstall(false, QString(), QStringLiteral("D:/Tools/MediaFlyouts")), InstallKind::Portable);
}

TEST(InstallSourceTest, RunningFromTheInstalledFolderIsAnInstallerInstall) {
    EXPECT_EQ(mf::classifyInstall(false, QStringLiteral("C:\\Program Files\\MediaFlyouts\\"),
                                  QStringLiteral("C:/Program Files/MediaFlyouts")),
              InstallKind::Installer);
    EXPECT_EQ(mf::classifyInstall(false, QStringLiteral("c:\\program files\\mediaflyouts"),
                                  QStringLiteral("C:/Program Files/MediaFlyouts/")),
              InstallKind::Installer);
}

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

TEST(InstallSourceTest, DetectionRunsWithoutCrashing) {
    const InstallKind kind = mf::detectInstallKind();
    EXPECT_TRUE(kind == InstallKind::Portable || kind == InstallKind::Unknown);
    // The test binary is never packaged.
    EXPECT_FALSE(mf::isPackagedProcess());
}

namespace {
const QString kRoaming = QStringLiteral("C:/Users/me/AppData/Roaming");
const QString kLocal = QStringLiteral("C:/Users/me/AppData/Local");
const QString kFamily = QStringLiteral("Publisher.MediaFlyouts_abc123");
const QString kCache = QStringLiteral("C:/Users/me/AppData/Local/Packages/Publisher.MediaFlyouts_abc123/LocalCache");
} // namespace

TEST(InstallSourceTest, PackagedRoamingPathMovesIntoTheLocalCache) {
    EXPECT_EQ(mf::packagedRedirectPath(QStringLiteral("C:/Users/me/AppData/Roaming/MediaFlyouts/MediaFlyouts.ini"),
                                       kRoaming, kLocal, kFamily),
              kCache + QStringLiteral("/Roaming/MediaFlyouts/MediaFlyouts.ini"));
    EXPECT_EQ(mf::packagedRedirectPath(QStringLiteral("C:\\Users\\me\\AppData\\Roaming\\MediaFlyouts"),
                                       QStringLiteral("C:\\Users\\me\\AppData\\Roaming\\"), kLocal, kFamily),
              kCache + QStringLiteral("/Roaming/MediaFlyouts"));
}

TEST(InstallSourceTest, PackagedLocalPathMovesIntoTheLocalCache) {
    EXPECT_EQ(mf::packagedRedirectPath(QStringLiteral("c:/users/me/appdata/local/MediaFlyouts/cache"), kRoaming, kLocal,
                                       kFamily),
              kCache + QStringLiteral("/Local/MediaFlyouts/cache"));
}

TEST(InstallSourceTest, PackagedPathLeavesRealLocationsAlone) {
    const QString packages = kLocal + QStringLiteral("/Packages/Other_xyz/LocalState/x.txt");
    EXPECT_EQ(mf::packagedRedirectPath(packages, kRoaming, kLocal, kFamily), packages);
    const QString programData = QStringLiteral("C:/ProgramData/MediaFlyouts/reports");
    EXPECT_EQ(mf::packagedRedirectPath(programData, kRoaming, kLocal, kFamily), programData);
    const QString sibling = QStringLiteral("C:/Users/me/AppData/LocalLow/x");
    EXPECT_EQ(mf::packagedRedirectPath(sibling, kRoaming, kLocal, kFamily), sibling);
    const QString roamingPrefix = QStringLiteral("C:/Users/me/AppData/Roaming2/x");
    EXPECT_EQ(mf::packagedRedirectPath(roamingPrefix, kRoaming, kLocal, kFamily), roamingPrefix);
}

TEST(InstallSourceTest, PackagedPathNeedsAFamilyName) {
    const QString path = QStringLiteral("C:/Users/me/AppData/Roaming/MediaFlyouts");
    EXPECT_EQ(mf::packagedRedirectPath(path, kRoaming, kLocal, QString()), path);
    EXPECT_EQ(mf::packagedRedirectPath(path, kRoaming, QString(), kFamily), path);
    EXPECT_EQ(mf::packagedRedirectPath(QString(), kRoaming, kLocal, kFamily), QString());
}

TEST(InstallSourceTest, HostVisiblePathIsIdentityWhenUnpackaged) {
    const QString path = QStringLiteral("C:/Users/me/AppData/Roaming/MediaFlyouts");
    EXPECT_EQ(mf::hostVisiblePath(path), path);
}
