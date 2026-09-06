#include <gtest/gtest.h>

#include <QFile>
#include <QString>

namespace {

QString manifestSource() {
    QFile file(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/packaging/msix/AppxManifest.xml"));
    EXPECT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    return QString::fromUtf8(file.readAll());
}

} // namespace

TEST(MsixManifestTest, DeclaresTheMediaFlyoutsStartupTask) {
    const QString source = manifestSource();
    EXPECT_TRUE(source.contains(QStringLiteral("xmlns:uap5=\"http://schemas.microsoft.com/appx/manifest/uap/windows10/5\"")));
    EXPECT_TRUE(source.contains(QStringLiteral("Category=\"windows.startupTask\"")));
    EXPECT_TRUE(source.contains(QStringLiteral("TaskId=\"MediaFlyoutsStartupTask\"")));
    EXPECT_TRUE(source.contains(QStringLiteral("Enabled=\"true\"")));
}
