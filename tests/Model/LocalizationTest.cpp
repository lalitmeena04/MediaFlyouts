#include <gtest/gtest.h>

#include <QStringList>

#include "Localization.h"

TEST(LocalizationTest, ShipsEveryRequestedLanguageWithEnglishFirst) {
    const QList<mf::LanguageInfo>& languages = mf::supportedLanguages();
    ASSERT_FALSE(languages.isEmpty());
    EXPECT_EQ(languages.first().code, QStringLiteral("en"));

    const QStringList expected = {QStringLiteral("en"), QStringLiteral("es"),    QStringLiteral("zh_CN"),
                                  QStringLiteral("ja"), QStringLiteral("de"),    QStringLiteral("fr"),
                                  QStringLiteral("ko"), QStringLiteral("pt_BR"), QStringLiteral("hi"),
                                  QStringLiteral("ar"), QStringLiteral("id")};
    QStringList codes;
    for(const mf::LanguageInfo& info : languages) {
        codes << info.code;
        EXPECT_FALSE(info.nativeName.isEmpty()) << info.code.toStdString();
    }
    EXPECT_EQ(codes, expected);
    for(const QString& code : expected)
        EXPECT_TRUE(mf::isSupportedLanguage(code)) << code.toStdString();
    EXPECT_FALSE(mf::isSupportedLanguage(QStringLiteral("xx")));
    EXPECT_FALSE(mf::isSupportedLanguage(QString()));
}

TEST(LocalizationTest, MatchLanguageNormalisesSeparatorsCaseAndScript) {
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("de")), QStringLiteral("de"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("pt-BR")), QStringLiteral("pt_BR"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("pt_br")), QStringLiteral("pt_BR"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("zh-Hans-CN")), QStringLiteral("zh_CN"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("ZH_CN")), QStringLiteral("zh_CN"));
}

TEST(LocalizationTest, MatchLanguageFallsBackToTheBaseLanguage) {
    // Regional variants that are not shipped pick the shipped sibling.
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("de-AT")), QStringLiteral("de"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("es-MX")), QStringLiteral("es"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("pt-PT")), QStringLiteral("pt_BR"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("zh")), QStringLiteral("zh_CN"));
    EXPECT_EQ(mf::matchLanguage(QStringLiteral("en-GB")), QStringLiteral("en"));
}

TEST(LocalizationTest, MatchLanguageReturnsEmptyForUnknownInput) {
    EXPECT_TRUE(mf::matchLanguage(QStringLiteral("xx")).isEmpty());
    EXPECT_TRUE(mf::matchLanguage(QStringLiteral("tlh-Latn")).isEmpty());
    EXPECT_TRUE(mf::matchLanguage(QString()).isEmpty());
    EXPECT_TRUE(mf::matchLanguage(QStringLiteral("  ")).isEmpty());
}

TEST(LocalizationTest, ResolvePrefersAnExplicitRequest) {
    const QStringList system = {QStringLiteral("fr-FR"), QStringLiteral("en-US")};
    EXPECT_EQ(mf::resolveLanguage(QStringLiteral("ja"), system), QStringLiteral("ja"));
    EXPECT_EQ(mf::resolveLanguage(QStringLiteral("pt-br"), system), QStringLiteral("pt_BR"));
}

TEST(LocalizationTest, ResolveFollowsTheSystemWhenRequestIsEmptyOrUnknown) {
    const QStringList system = {QStringLiteral("nl-NL"), QStringLiteral("ko-KR"), QStringLiteral("en-US")};
    EXPECT_EQ(mf::resolveLanguage(QString(), system), QStringLiteral("ko"));
    EXPECT_EQ(mf::resolveLanguage(QStringLiteral("xx"), system), QStringLiteral("ko"));
}

TEST(LocalizationTest, ResolveFallsBackToEnglishWhenNothingMatches) {
    EXPECT_EQ(mf::resolveLanguage(QString(), {}), QStringLiteral("en"));
    EXPECT_EQ(mf::resolveLanguage(QString(), {QStringLiteral("nl"), QStringLiteral("sv-SE")}), QStringLiteral("en"));
    EXPECT_EQ(mf::defaultLanguage(), QStringLiteral("en"));
}

TEST(LocalizationTest, TranslationFileNameUsesTheCode) {
    EXPECT_EQ(mf::translationFileName(QStringLiteral("pt_BR")), QStringLiteral("MediaFlyouts_pt_BR"));
    EXPECT_EQ(mf::translationFileName(QStringLiteral("de")), QStringLiteral("MediaFlyouts_de"));
}

TEST(LocalizationTest, RightToLeftOnlyForRtlScripts) {
    EXPECT_TRUE(mf::isRightToLeft(QStringLiteral("ar")));
    EXPECT_TRUE(mf::isRightToLeft(QStringLiteral("ar_EG")));
    EXPECT_FALSE(mf::isRightToLeft(QStringLiteral("en")));
    EXPECT_FALSE(mf::isRightToLeft(QStringLiteral("hi")));
    EXPECT_FALSE(mf::isRightToLeft(QString()));
}

TEST(LocalizationTest, FallbackFontsCoverDevanagari) {
    const QStringList families = mf::fallbackFontFamilies();
    EXPECT_TRUE(families.contains(QStringLiteral("Nirmala UI")));
    for(const QString& family : families)
        EXPECT_FALSE(family.trimmed().isEmpty());
}
