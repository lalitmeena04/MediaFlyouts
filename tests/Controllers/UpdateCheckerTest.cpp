#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QString>

#include "UpdateChecker.h"

namespace {
QJsonObject asset(const QString& name, const QString& url, qint64 size = 1024, const QString& digest = QString()) {
    QJsonObject obj;
    obj.insert(QStringLiteral("name"), name);
    obj.insert(QStringLiteral("browser_download_url"), url);
    obj.insert(QStringLiteral("size"), size);
    if(!digest.isEmpty())
        obj.insert(QStringLiteral("digest"), digest);
    return obj;
}

const QString kSetupUrl = QStringLiteral(
    "https://github.com/scleaverzer0ne/MediaFlyouts-Releases/releases/download/v1.5.0/MediaFlyouts-1.5.0-setup.exe");
const QString kSha = QStringLiteral("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");
} // namespace

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
    EXPECT_FALSE(u.installerAvailable());
    EXPECT_FALSE(u.downloading());
    EXPECT_FALSE(u.downloaded());
    EXPECT_DOUBLE_EQ(u.downloadProgress(), 0.0);
    EXPECT_TRUE(u.downloadError().isEmpty());
    EXPECT_TRUE(u.downloadedFile().isEmpty());
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
        QStringLiteral("https://github.com/scleaverzer0ne/MediaFlyouts-Releases/releases/tag/v1.4.1")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QStringLiteral("http://github.com/owner/repo")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QStringLiteral("https://github.com.evil.test/owner/repo")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QStringLiteral("file:///C:/Windows/System32/cmd.exe")));
    EXPECT_FALSE(UpdateChecker::isTrustedReleaseUrl(QString()));
}

// Only a plain setup file name may be written to disk.
TEST(UpdateCheckerTest, InstallerNameMustBeAPlainSetupExecutable) {
    EXPECT_TRUE(UpdateChecker::isInstallerName(QStringLiteral("MediaFlyouts-1.5.0-setup.exe")));
    EXPECT_TRUE(UpdateChecker::isInstallerName(QStringLiteral("MediaFlyouts-1.5.0-rc1-Setup.EXE")));
    EXPECT_FALSE(UpdateChecker::isInstallerName(QStringLiteral("MediaFlyouts-1.5.0-win64.zip")));
    EXPECT_FALSE(UpdateChecker::isInstallerName(QStringLiteral("MediaFlyouts-1.5.0.msix")));
    EXPECT_FALSE(UpdateChecker::isInstallerName(QStringLiteral("..\\evil-setup.exe")));
    EXPECT_FALSE(UpdateChecker::isInstallerName(QStringLiteral("dir/MediaFlyouts-setup.exe")));
    EXPECT_FALSE(UpdateChecker::isInstallerName(QStringLiteral("setup.exe")));
    EXPECT_FALSE(UpdateChecker::isInstallerName(QString()));
}

TEST(UpdateCheckerTest, ParsesOnlySha256Digests) {
    EXPECT_EQ(UpdateChecker::sha256FromDigest(QStringLiteral("sha256:") + kSha.toUpper()), kSha);
    EXPECT_TRUE(UpdateChecker::sha256FromDigest(QStringLiteral("md5:0123456789abcdef0123456789abcdef")).isEmpty());
    EXPECT_TRUE(UpdateChecker::sha256FromDigest(QStringLiteral("sha256:tooshort")).isEmpty());
    EXPECT_TRUE(UpdateChecker::sha256FromDigest(QString()).isEmpty());
}

TEST(UpdateCheckerTest, PicksTheSetupAssetAndSkipsTheRest) {
    QJsonArray assets;
    assets.append(asset(QStringLiteral("MediaFlyouts-1.5.0-win64.zip"),
                        QStringLiteral("https://github.com/o/r/releases/download/v1.5.0/MediaFlyouts-1.5.0-win64.zip")));
    assets.append(asset(QStringLiteral("MediaFlyouts-1.5.0-setup.exe"), kSetupUrl, 4096,
                        QStringLiteral("sha256:") + kSha));
    assets.append(asset(QStringLiteral("MediaFlyouts-1.5.0.msix"),
                        QStringLiteral("https://github.com/o/r/releases/download/v1.5.0/MediaFlyouts-1.5.0.msix")));

    const UpdateChecker::InstallerAsset picked = UpdateChecker::pickInstallerAsset(assets);
    ASSERT_TRUE(picked.valid());
    EXPECT_EQ(picked.name, QStringLiteral("MediaFlyouts-1.5.0-setup.exe"));
    EXPECT_EQ(picked.url, kSetupUrl);
    EXPECT_EQ(picked.size, 4096);
    EXPECT_EQ(picked.sha256, kSha);
}

// A setup file hosted off GitHub is never downloaded.
TEST(UpdateCheckerTest, IgnoresSetupAssetsFromUntrustedHosts) {
    QJsonArray assets;
    assets.append(asset(QStringLiteral("MediaFlyouts-1.5.0-setup.exe"),
                        QStringLiteral("https://evil.test/MediaFlyouts-1.5.0-setup.exe")));
    EXPECT_FALSE(UpdateChecker::pickInstallerAsset(assets).valid());
    EXPECT_FALSE(UpdateChecker::pickInstallerAsset(QJsonArray()).valid());
}

TEST(UpdateCheckerTest, DigestWinsOverSizeWhenVerifying) {
    UpdateChecker::InstallerAsset withDigest;
    withDigest.url = kSetupUrl;
    withDigest.size = 10;
    withDigest.sha256 = kSha;
    EXPECT_TRUE(UpdateChecker::matchesAsset(withDigest, 999, kSha.toUpper()));
    EXPECT_FALSE(UpdateChecker::matchesAsset(withDigest, 10, QStringLiteral("deadbeef")));

    UpdateChecker::InstallerAsset sizeOnly;
    sizeOnly.url = kSetupUrl;
    sizeOnly.size = 10;
    EXPECT_TRUE(UpdateChecker::matchesAsset(sizeOnly, 10, QString()));
    EXPECT_FALSE(UpdateChecker::matchesAsset(sizeOnly, 11, QString()));

    UpdateChecker::InstallerAsset unknown;
    unknown.url = kSetupUrl;
    EXPECT_TRUE(UpdateChecker::matchesAsset(unknown, 1, QString()));
    EXPECT_FALSE(UpdateChecker::matchesAsset(unknown, 0, QString()));
}

// Store copies update through the Store. ZIP copies have nothing to run the setup over.
TEST(UpdateCheckerTest, OnlyInstallerInstallsMaySelfUpdate) {
    EXPECT_TRUE(UpdateChecker::selfUpdateAllowed(mf::InstallKind::Installer));
    EXPECT_FALSE(UpdateChecker::selfUpdateAllowed(mf::InstallKind::Store));
    EXPECT_FALSE(UpdateChecker::selfUpdateAllowed(mf::InstallKind::Portable));
    EXPECT_FALSE(UpdateChecker::selfUpdateAllowed(mf::InstallKind::Unknown));
}

TEST(UpdateCheckerTest, InstallKindIsExposedToQml) {
    UpdateChecker u;
    QSignalSpy kindChanged(&u, &UpdateChecker::installKindChanged);
    u.setInstallKind(mf::InstallKind::Store);
    EXPECT_EQ(u.installKind(), mf::InstallKind::Store);
    EXPECT_EQ(u.installKindId(), QStringLiteral("store"));
    EXPECT_EQ(u.installKindLabel(), QStringLiteral("Microsoft Store"));
    EXPECT_EQ(kindChanged.count(), 1);
    // Same value again is silent.
    u.setInstallKind(mf::InstallKind::Store);
    EXPECT_EQ(kindChanged.count(), 1);
    // Store copies never offer the in-app installer, update or not.
    EXPECT_FALSE(u.installerAvailable());
}

// Nothing to run before a download. Nothing to fetch before a check.
TEST(UpdateCheckerTest, DownloadAndInstallAreNoOpsWithoutAnUpdate) {
    UpdateChecker u;
    QSignalSpy started(&u, &UpdateChecker::installStarted);
    QSignalSpy found(&u, &UpdateChecker::updateFound);

    u.download();
    EXPECT_FALSE(u.downloading());
    u.install();
    EXPECT_EQ(started.count(), 0);
    u.notifyUpdate();
    EXPECT_EQ(found.count(), 0);
    u.cancelDownload();
    EXPECT_TRUE(u.downloadError().isEmpty());
}
