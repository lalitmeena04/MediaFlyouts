#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace mf {

// One UI language the app ships translations for.
struct LanguageInfo {
        QString code;       // Qt locale code used in file names, e.g. "pt_BR".
        QString nativeName; // Name shown in the picker, written in that language.
};

// Code of the source language. Used when nothing else matches.
QString defaultLanguage();

// Every shipped language, in picker order. English first.
const QList<LanguageInfo>& supportedLanguages();

bool isSupportedLanguage(const QString& code);

// Turns "pt-BR", "pt_br" or "zh-Hans-CN" into a shipped code when one matches.
// Exact match wins. Else the first shipped code with the same base language.
// Returns an empty string when nothing matches.
QString matchLanguage(const QString& candidate);

// Picks the language to load.
// requested: a code from Settings. Empty means follow the system.
// systemUiLanguages: QLocale::system().uiLanguages(), most preferred first.
// Falls back to defaultLanguage() when nothing matches.
QString resolveLanguage(const QString& requested, const QStringList& systemUiLanguages);

// Base name of the app translation file for a code, e.g. "MediaFlyouts_pt_BR".
QString translationFileName(const QString& code);

// True for languages written right to left.
bool isRightToLeft(const QString& code);

// Font families Qt tries before the system fallback when the UI font lacks a
// glyph. Applied for every language, so text in these scripts looks the same
// everywhere. Families that are not installed are skipped by Qt.
QStringList fallbackFontFamilies();

} // namespace mf
