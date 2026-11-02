#include <gtest/gtest.h>

#include <QFile>
#include <QRegularExpression>
#include <QString>

#include "Version.h"
#include "WhatsNew.h"

TEST(WhatsNewTest, BaseVersionDropsSuffixAndPrefix) {
    EXPECT_EQ(mf::baseVersion(QStringLiteral("1.7.0")), QStringLiteral("1.7.0"));
    EXPECT_EQ(mf::baseVersion(QStringLiteral("1.7.0-263.63b8254")), QStringLiteral("1.7.0"));
    EXPECT_EQ(mf::baseVersion(QStringLiteral("v1.7.0")), QStringLiteral("1.7.0"));
    EXPECT_EQ(mf::baseVersion(QStringLiteral(" 1.7.0-dev ")), QStringLiteral("1.7.0"));
    EXPECT_TRUE(mf::baseVersion(QString()).isEmpty());
}

TEST(WhatsNewTest, ShowsOnlyWhenADifferentVersionRanBefore) {
    EXPECT_FALSE(mf::shouldShowWhatsNew(QString(), QStringLiteral("1.7.0")));
    EXPECT_FALSE(mf::shouldShowWhatsNew(QStringLiteral("1.7.0"), QStringLiteral("1.7.0")));
    EXPECT_FALSE(mf::shouldShowWhatsNew(QStringLiteral("1.7.0"), QStringLiteral("1.7.0-12.abc")));
    EXPECT_TRUE(mf::shouldShowWhatsNew(QStringLiteral("1.6.0"), QStringLiteral("1.7.0")));
    EXPECT_TRUE(mf::shouldShowWhatsNew(QStringLiteral("1.8.0"), QStringLiteral("1.7.0")));
    EXPECT_FALSE(mf::shouldShowWhatsNew(QStringLiteral("1.6.0"), QString()));
}

TEST(WhatsNewTest, ParsesTheVersionLineAndStripsIt) {
    const QString text = QStringLiteral("Version 1.7.0\n\n- First\n- Second");
    EXPECT_EQ(mf::whatsNewVersion(text), QStringLiteral("1.7.0"));
    EXPECT_EQ(mf::whatsNewBody(text), QStringLiteral("- First\n- Second"));

    const QString noHeader = QStringLiteral("- Only bullets");
    EXPECT_TRUE(mf::whatsNewVersion(noHeader).isEmpty());
    EXPECT_EQ(mf::whatsNewBody(noHeader), noHeader);

    EXPECT_TRUE(mf::whatsNewVersion(QStringLiteral("Version 1.7\n- x")).isEmpty());
    EXPECT_TRUE(mf::whatsNewVersion(QString()).isEmpty());
}

// WHATS_NEW.md is pasted into the Store "What's new" field as plain text.
TEST(WhatsNewTest, BundledNotesMatchTheBuildAndStayStoreSafe) {
    const QString text = mf::whatsNewText();
    ASSERT_FALSE(text.isEmpty()) << "WHATS_NEW.md is not embedded";
    EXPECT_EQ(mf::whatsNewVersion(text), mf::baseVersion(QStringLiteral(MEDIAFLYOUTS_VERSION)))
        << "WHATS_NEW.md first line must be 'Version <VERSION.txt>'";
    EXPECT_LE(text.size(), 1500) << "Partner Center limits the field to 1500 characters";
    EXPECT_FALSE(text.contains(QStringLiteral("**"))) << "no markdown emphasis, the Store shows it literally";
    EXPECT_FALSE(text.contains(QStringLiteral("](")) ) << "no markdown links";
    EXPECT_FALSE(text.section(QLatin1Char('\n'), 1).contains(QRegularExpression(QStringLiteral("^#"),
                                                                                  QRegularExpression::MultilineOption)))
        << "no markdown headings";
    const QString body = mf::whatsNewBody(text);
    EXPECT_TRUE(body.startsWith(QStringLiteral("- "))) << "body must be a bullet list";
}

TEST(WhatsNewTest, SourceFileAndResourceAreTheSame) {
    QFile source(QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR "/WHATS_NEW.md"));
    ASSERT_TRUE(source.open(QIODevice::ReadOnly | QIODevice::Text));
    EXPECT_EQ(QString::fromUtf8(source.readAll()).trimmed(), mf::whatsNewText());
}
