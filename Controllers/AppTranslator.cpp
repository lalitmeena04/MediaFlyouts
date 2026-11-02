#include "AppTranslator.h"

#include <QCoreApplication>
#include <QFont>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QQmlEngine>
#include <QVariantMap>

#include "Localization.h"
#include "Logging.h"

AppTranslator::AppTranslator(QObject* parent) : QObject(parent) {
    installFallbackFonts();
}

AppTranslator::~AppTranslator() {
    QCoreApplication::removeTranslator(&m_app);
    QCoreApplication::removeTranslator(&m_qt);
}

void AppTranslator::setEngine(QQmlEngine* engine) {
    m_engine = engine;
}

QString AppTranslator::language() const {
    return m_language;
}

QVariantList AppTranslator::languages() const {
    QVariantList list;
    // Empty code = follow the system. QML supplies the translated label.
    list << QVariantMap{{QStringLiteral("code"), QString()}, {QStringLiteral("name"), QString()}};
    for(const mf::LanguageInfo& info : mf::supportedLanguages())
        list << QVariantMap{{QStringLiteral("code"), info.code}, {QStringLiteral("name"), info.nativeName}};
    return list;
}

bool AppTranslator::rightToLeft() const {
    return mf::isRightToLeft(m_language);
}

QString AppTranslator::appTranslationPath(const QString& code) {
    return QStringLiteral(":/i18n/") + mf::translationFileName(code) + QStringLiteral(".qm");
}

void AppTranslator::installFallbackFonts() {
    // Needs a GUI application. Headless tools keep the plain core font.
    if(!qobject_cast<QGuiApplication*>(QCoreApplication::instance()))
        return;
    QFont font = QGuiApplication::font();
    QStringList families = font.families();
    if(families.isEmpty())
        families << font.family();
    bool changed = false;
    for(const QString& family : mf::fallbackFontFamilies()) {
        if(families.contains(family, Qt::CaseInsensitive))
            continue;
        families << family;
        changed = true;
    }
    if(!changed)
        return;
    font.setFamilies(families);
    QGuiApplication::setFont(font);
    TRC_INFO("i18n: app font families [{}]", families.join(QStringLiteral(", ")).toStdString());
}

bool AppTranslator::loadAppTranslation(const QString& code) {
    QCoreApplication::removeTranslator(&m_app);
    // English is the source language. Its file is empty and does not need to load.
    if(code == mf::defaultLanguage())
        return true;
    const QString path = appTranslationPath(code);
    if(!m_app.load(path)) {
        TRC_WARN("i18n: app translation missing: {}", path.toStdString());
        return false;
    }
    QCoreApplication::installTranslator(&m_app);
    return true;
}

bool AppTranslator::loadQtTranslation(const QString& code) {
    QCoreApplication::removeTranslator(&m_qt);
    // Qt ships its own strings (dialog buttons etc.) next to the runtime.
    const QString dir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if(!m_qt.load(QLocale(code), QStringLiteral("qtbase"), QStringLiteral("_"), dir)) {
        TRC_DEBUG("i18n: no qtbase translation for {} in {}", code.toStdString(), dir.toStdString());
        return false;
    }
    QCoreApplication::installTranslator(&m_qt);
    return true;
}

QString AppTranslator::apply(const QString& requested) {
    // Settings::changed fires for every setting. Skip the reload when nothing moved.
    if(m_applied && requested == m_requested)
        return m_language;
    m_applied = true;
    m_requested = requested;

    const QStringList systemLanguages = QLocale::system().uiLanguages();
    const QString code = mf::resolveLanguage(requested, systemLanguages);
    TRC_INFO("i18n: requested '{}' system [{}] -> {}", requested.toStdString(),
             systemLanguages.join(QStringLiteral(", ")).toStdString(), code.toStdString());

    const bool appLoaded = loadAppTranslation(code);
    const bool qtLoaded = loadQtTranslation(code);
    TRC_DEBUG("i18n: loaded app={} qt={}", appLoaded, qtLoaded);
    QLocale::setDefault(QLocale(code));

    const bool changed = m_language != code;
    m_language = code;
    if(changed) {
        if(m_engine)
            m_engine->retranslate();
        emit languageChanged();
    }
    return code;
}
