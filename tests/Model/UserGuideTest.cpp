#include <gtest/gtest.h>

#include <QFile>
#include <QString>

#include "UserGuide.h"

TEST(UserGuideTest, BundledGuideMatchesTheSourceDocument) {
    const QString text = mf::userGuideText();
    ASSERT_FALSE(text.isEmpty()) << "user-guide.md is not embedded";

    QFile source(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/docs/user/user-guide.md"));
    ASSERT_TRUE(source.open(QIODevice::ReadOnly | QIODevice::Text));
    EXPECT_EQ(text, QString::fromUtf8(source.readAll()));
    EXPECT_TRUE(text.contains(QStringLiteral("## Install the browser extension")));
}
