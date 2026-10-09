#include <gtest/gtest.h>

#include <QFile>
#include <QRegularExpression>
#include <QString>

namespace {

QString manifestSource() {
    QFile file(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/packaging/msix/AppxManifest.xml"));
    EXPECT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    return QString::fromUtf8(file.readAll());
}

// Same patterns tools/stage-msix.ps1 uses to stamp the version and Store identity.
QString injectVersion(QString source, const QString& version) {
    static const QRegularExpression pattern(QStringLiteral("\\bVersion=\"\\d+\\.\\d+\\.\\d+\\.\\d+\""));
    return source.replace(pattern, QStringLiteral("Version=\"%1\"").arg(version));
}

QString injectIdentity(QString source, const QString& name, const QString& publisher) {
    static const QRegularExpression namePattern(QStringLiteral("\\bName=\"MediaFlyouts\""));
    static const QRegularExpression publisherPattern(QStringLiteral("Publisher=\"CN=MediaFlyouts\""));
    source.replace(namePattern, QStringLiteral("Name=\"%1\"").arg(name));
    return source.replace(publisherPattern, QStringLiteral("Publisher=\"%1\"").arg(publisher));
}

} // namespace

TEST(MsixManifestTest, DeclaresTheMediaFlyoutsStartupTask) {
    const QString source = manifestSource();
    EXPECT_TRUE(source.contains(QStringLiteral("xmlns:uap5=\"http://schemas.microsoft.com/appx/manifest/uap/windows10/5\"")));
    EXPECT_TRUE(source.contains(QStringLiteral("Category=\"windows.startupTask\"")));
    EXPECT_TRUE(source.contains(QStringLiteral("TaskId=\"MediaFlyoutsStartupTask\"")));
    EXPECT_TRUE(source.contains(QStringLiteral("Enabled=\"true\"")));
}

TEST(MsixManifestTest, TargetsAStoreAcceptedMinimumWindowsVersion) {
    // Partner Center rejects MinVersion <= 10.0.17134.0.
    EXPECT_TRUE(manifestSource().contains(QStringLiteral("MinVersion=\"10.0.19041.0\"")));
}

TEST(MsixManifestTest, UsesThePartnerCenterPublisherDisplayName) {
    EXPECT_TRUE(manifestSource().contains(QStringLiteral("<PublisherDisplayName>ScleaverZer0ne</PublisherDisplayName>")));
}

TEST(MsixManifestTest, UsesOnlyTheReservedStoreDisplayName) {
    // Partner Center rejects any DisplayName that is not a reserved app name.
    const QString source = manifestSource();
    EXPECT_TRUE(source.contains(QStringLiteral("<DisplayName>Media Flyouts</DisplayName>")));
    EXPECT_FALSE(source.contains(QStringLiteral("<DisplayName>MediaFlyouts</DisplayName>")));
    EXPECT_FALSE(source.contains(QStringLiteral("DisplayName=\"MediaFlyouts\"")));
}

TEST(MsixManifestTest, VersionInjectionLeavesMinVersionAlone) {
    const QString stamped = injectVersion(manifestSource(), QStringLiteral("9.8.7.6"));
    EXPECT_TRUE(stamped.contains(QStringLiteral("Version=\"9.8.7.6\"")));
    EXPECT_TRUE(stamped.contains(QStringLiteral("MinVersion=\"10.0.19041.0\"")));
    EXPECT_TRUE(stamped.contains(QStringLiteral("MaxVersionTested=\"10.0.26100.0\"")));
}

TEST(MsixManifestTest, IdentityInjectionTouchesOnlyTheIdentityElement) {
    const QString stamped = injectIdentity(manifestSource(), QStringLiteral("Pub.MediaFlyouts"),
                                           QStringLiteral("CN=00000000-0000-0000-0000-000000000000"));
    EXPECT_TRUE(stamped.contains(QStringLiteral("Name=\"Pub.MediaFlyouts\"")));
    EXPECT_TRUE(stamped.contains(QStringLiteral("Publisher=\"CN=00000000-0000-0000-0000-000000000000\"")));
    EXPECT_FALSE(stamped.contains(QStringLiteral("Publisher=\"CN=MediaFlyouts\"")));
    // Display names, device family and capability names keep their values.
    EXPECT_TRUE(stamped.contains(QStringLiteral("DisplayName=\"Media Flyouts\"")));
    EXPECT_TRUE(stamped.contains(QStringLiteral("<DisplayName>Media Flyouts</DisplayName>")));
    EXPECT_TRUE(stamped.contains(QStringLiteral("Name=\"Windows.Desktop\"")));
    EXPECT_TRUE(stamped.contains(QStringLiteral("Name=\"runFullTrust\"")));
}
