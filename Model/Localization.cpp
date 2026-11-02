#include "Localization.h"

#include "Logging.h"

namespace mf {

namespace {

// Normalises "pt-BR" / "pt_br" / "zh-Hans-CN" into ["pt", "BR"] style parts.
// Script subtags (4 letters) are dropped. Case is fixed to Qt convention.
QStringList languageParts(const QString& candidate) {
    QString s = candidate.trimmed();
    s.replace(QLatin1Char('-'), QLatin1Char('_'));
    QStringList parts;
    for(const QString& raw : s.split(QLatin1Char('_'), Qt::SkipEmptyParts)) {
        if(parts.isEmpty())
            parts << raw.toLower();
        else if(raw.size() == 4)
            continue; // Script, e.g. "Hans".
        else
            parts << raw.toUpper();
    }
    return parts;
}

QString baseLanguage(const QString& code) {
    return code.section(QLatin1Char('_'), 0, 0);
}

} // namespace

QString defaultLanguage() {
    return QStringLiteral("en");
}

const QList<LanguageInfo>& supportedLanguages() {
    static const QList<LanguageInfo> languages = {
        {QStringLiteral("en"), QStringLiteral("English")},
        {QStringLiteral("es"), QStringLiteral("Español")},
        {QStringLiteral("zh_CN"), QStringLiteral("简体中文")},
        {QStringLiteral("ja"), QStringLiteral("日本語")},
        {QStringLiteral("de"), QStringLiteral("Deutsch")},
        {QStringLiteral("fr"), QStringLiteral("Français")},
        {QStringLiteral("ko"), QStringLiteral("한국어")},
        {QStringLiteral("pt_BR"), QStringLiteral("Português (Brasil)")},
        {QStringLiteral("hi"), QStringLiteral("हिन्दी")},
        {QStringLiteral("ar"), QStringLiteral("العربية")},
        {QStringLiteral("id"), QStringLiteral("Bahasa Indonesia")},
    };
    return languages;
}

bool isSupportedLanguage(const QString& code) {
    for(const LanguageInfo& info : supportedLanguages())
        if(info.code == code)
            return true;
    return false;
}

QString matchLanguage(const QString& candidate) {
    const QStringList parts = languageParts(candidate);
    if(parts.isEmpty())
        return QString();
    const QString exact = parts.join(QLatin1Char('_'));
    if(isSupportedLanguage(exact))
        return exact;
    const QString base = parts.first();
    for(const LanguageInfo& info : supportedLanguages())
        if(baseLanguage(info.code) == base)
            return info.code;
    return QString();
}

QString resolveLanguage(const QString& requested, const QStringList& systemUiLanguages) {
    if(!requested.isEmpty()) {
        const QString match = matchLanguage(requested);
        if(!match.isEmpty())
            return match;
        TRC_WARN("i18n: requested language '{}' is not shipped, following the system", requested.toStdString());
    }
    for(const QString& candidate : systemUiLanguages) {
        const QString match = matchLanguage(candidate);
        if(!match.isEmpty())
            return match;
    }
    return defaultLanguage();
}

QString translationFileName(const QString& code) {
    return QStringLiteral("MediaFlyouts_") + code;
}

bool isRightToLeft(const QString& code) {
    static const QStringList rtl = {QStringLiteral("ar"), QStringLiteral("he"), QStringLiteral("fa"),
                                    QStringLiteral("ur")};
    return rtl.contains(baseLanguage(code));
}

QStringList fallbackFontFamilies() {
    // Devanagari: the system fallback for Segoe UI draws glyphs about 30 % smaller
    // than Latin at the same pixel size. Nirmala UI is the Windows UI font for the
    // script and matches Segoe UI metrics.
    return {QStringLiteral("Nirmala UI")};
}

} // namespace mf
