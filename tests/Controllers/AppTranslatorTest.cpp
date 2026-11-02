#include <gtest/gtest.h>

#include <memory>

#include <QCoreApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QVariantMap>

#include "AppTranslator.h"
#include "Capabilities.h"
#include "Localization.h"

namespace {

// Each test leaves the process back in English so later tests see source text.
class AppTranslatorTest : public ::testing::Test {
    protected:
        void TearDown() override {
            AppTranslator reset;
            reset.apply(QStringLiteral("en"));
        }
};

} // namespace

TEST_F(AppTranslatorTest, LanguageListStartsWithSystemDefaultThenEveryShippedLanguage) {
    AppTranslator translator;
    const QVariantList list = translator.languages();
    ASSERT_EQ(list.size(), mf::supportedLanguages().size() + 1);

    const QVariantMap first = list.first().toMap();
    EXPECT_TRUE(first.value(QStringLiteral("code")).toString().isEmpty());

    for(int i = 0; i < mf::supportedLanguages().size(); ++i) {
        const QVariantMap entry = list.at(i + 1).toMap();
        EXPECT_EQ(entry.value(QStringLiteral("code")).toString(), mf::supportedLanguages().at(i).code);
        EXPECT_EQ(entry.value(QStringLiteral("name")).toString(), mf::supportedLanguages().at(i).nativeName);
    }
}

TEST_F(AppTranslatorTest, EveryShippedLanguageHasAnEmbeddedTranslationFile) {
    for(const mf::LanguageInfo& info : mf::supportedLanguages()) {
        if(info.code == mf::defaultLanguage())
            continue; // English is the source language. It ships no .qm.
        const QString path = AppTranslator::appTranslationPath(info.code);
        EXPECT_TRUE(QFile::exists(path)) << path.toStdString();
    }
}

TEST_F(AppTranslatorTest, ApplyLoadsTheRequestedLanguageAndTranslatesCppStrings) {
    AppTranslator translator;
    QSignalSpy changed(&translator, &AppTranslator::languageChanged);

    EXPECT_EQ(translator.apply(QStringLiteral("de")), QStringLiteral("de"));
    EXPECT_EQ(translator.language(), QStringLiteral("de"));
    EXPECT_EQ(changed.count(), 1);
    EXPECT_FALSE(translator.rightToLeft());

    Capabilities caps;
    caps.setAudioAvailable(false);
    EXPECT_EQ(caps.unavailableNames(), QStringList{QStringLiteral("Audiogeräte")});
    EXPECT_EQ(QCoreApplication::translate("TrayController", "Settings"), QStringLiteral("Einstellungen"));
}

TEST_F(AppTranslatorTest, SwitchingBackToEnglishRestoresSourceText) {
    AppTranslator translator;
    translator.apply(QStringLiteral("fr"));
    EXPECT_EQ(QCoreApplication::translate("TrayController", "Settings"), QStringLiteral("Paramètres"));

    translator.apply(QStringLiteral("en"));
    EXPECT_EQ(translator.language(), QStringLiteral("en"));
    EXPECT_EQ(QCoreApplication::translate("TrayController", "Settings"), QStringLiteral("Settings"));
}

TEST_F(AppTranslatorTest, ReapplyingTheSameRequestDoesNotEmitAgain) {
    AppTranslator translator;
    QSignalSpy changed(&translator, &AppTranslator::languageChanged);
    translator.apply(QStringLiteral("es"));
    translator.apply(QStringLiteral("es"));
    translator.apply(QStringLiteral("es"));
    EXPECT_EQ(changed.count(), 1);
}

TEST_F(AppTranslatorTest, UnknownRequestFallsBackToAShippedLanguage) {
    AppTranslator translator;
    const QString code = translator.apply(QStringLiteral("tlh"));
    EXPECT_TRUE(mf::isSupportedLanguage(code)) << code.toStdString();
    EXPECT_EQ(translator.language(), code);
}

TEST_F(AppTranslatorTest, EmptyRequestFollowsTheSystemAndAlwaysYieldsAShippedCode) {
    AppTranslator translator;
    const QString code = translator.apply(QString());
    EXPECT_FALSE(code.isEmpty());
    EXPECT_TRUE(mf::isSupportedLanguage(code)) << code.toStdString();
}

TEST_F(AppTranslatorTest, ArabicReportsRightToLeft) {
    AppTranslator translator;
    translator.apply(QStringLiteral("ar"));
    EXPECT_TRUE(translator.rightToLeft());
}

TEST_F(AppTranslatorTest, RetranslatesQmlBindingsWhenAnEngineIsSet) {
    // qsTr() takes its context from the QML file name. "TrayController.qml" maps to
    // the TrayController context, which ships "Settings" in every language.
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick\nQtObject { property string text: qsTr(\"Settings\") }",
                      QUrl(QStringLiteral("qrc:/test/TrayController.qml")));
    std::unique_ptr<QObject> object(component.create());
    ASSERT_TRUE(object) << component.errorString().toStdString();
    EXPECT_EQ(object->property("text").toString(), QStringLiteral("Settings"));

    AppTranslator translator;
    translator.setEngine(&engine);
    translator.apply(QStringLiteral("de"));
    EXPECT_EQ(object->property("text").toString(), QStringLiteral("Einstellungen"));

    translator.apply(QStringLiteral("ja"));
    EXPECT_EQ(object->property("text").toString(), QStringLiteral("設定"));

    translator.apply(QStringLiteral("en"));
    EXPECT_EQ(object->property("text").toString(), QStringLiteral("Settings"));
}

TEST_F(AppTranslatorTest, LoadsQtsOwnTranslationsWhenTheyAreInstalled) {
    const QString dir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if(!QFile::exists(dir + QStringLiteral("/qtbase_de.qm")))
        GTEST_SKIP() << "qtbase_de.qm not found in " << dir.toStdString();

    AppTranslator translator;
    translator.apply(QStringLiteral("de"));
    // "Cancel" in the QPlatformTheme context comes from qtbase, not from the app file.
    EXPECT_EQ(QCoreApplication::translate("QPlatformTheme", "Cancel"), QStringLiteral("Abbrechen"));

    translator.apply(QStringLiteral("en"));
    EXPECT_EQ(QCoreApplication::translate("QPlatformTheme", "Cancel"), QStringLiteral("Cancel"));
}

TEST_F(AppTranslatorTest, ApplySetsTheDefaultLocale) {
    AppTranslator translator;
    translator.apply(QStringLiteral("pt_BR"));
    EXPECT_EQ(QLocale().language(), QLocale::Portuguese);
    EXPECT_EQ(QLocale().territory(), QLocale::Brazil);

    translator.apply(QStringLiteral("en"));
    EXPECT_EQ(QLocale().language(), QLocale::English);
}

TEST_F(AppTranslatorTest, AppFontGainsTheScriptFallbackFamiliesOnce) {
    const QString primary = QGuiApplication::font().families().value(0, QGuiApplication::font().family());
    AppTranslator first;
    AppTranslator second;
    const QStringList families = QGuiApplication::font().families();
    ASSERT_FALSE(families.isEmpty());
    EXPECT_EQ(families.first(), primary);
    for(const QString& fallback : mf::fallbackFontFamilies())
        EXPECT_EQ(families.count(fallback), 1) << fallback.toStdString();
}

TEST_F(AppTranslatorTest, DevanagariRendersAtLatinSizeWithTheFallbackFont) {
    // The system fallback for the UI font draws Devanagari about 30 % smaller than
    // Latin. With Nirmala UI in the chain the two scripts have the same height.
    if(!QFontDatabase::families().contains(QStringLiteral("Nirmala UI")))
        GTEST_SKIP() << "Nirmala UI is not installed";
    AppTranslator translator;
    QFont font = QGuiApplication::font();
    font.setPixelSize(20);
    const QFontMetricsF metrics(font);
    const qreal latin = metrics.tightBoundingRect(QStringLiteral("Language Settings")).height();
    const qreal devanagari = metrics.tightBoundingRect(QStringLiteral("भाषा सेटिंग्स")).height();
    EXPECT_GE(devanagari, latin * 0.9) << "devanagari " << devanagari << " vs latin " << latin;
}
