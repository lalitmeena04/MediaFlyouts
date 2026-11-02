#pragma once

#include <QObject>
#include <QString>
#include <QTranslator>
#include <QVariantList>

class QQmlEngine;

// Loads the app and Qt translations for one language and swaps them at run time.
// QML text bound through qsTr() refreshes when the engine retranslates.
// C++ owners of static text listen to languageChanged() and set it again.
// Construction also installs the script fallback fonts on the application font.
class AppTranslator : public QObject {
        Q_OBJECT
        // Effective language code, e.g. "de". Never empty after apply().
        Q_PROPERTY(QString language READ language NOTIFY languageChanged)
        // Picker entries. Each item has "code" and "name". The first entry is the system default with an empty code.
        Q_PROPERTY(QVariantList languages READ languages CONSTANT)
        Q_PROPERTY(bool rightToLeft READ rightToLeft NOTIFY languageChanged)

    public:
        explicit AppTranslator(QObject* parent = nullptr);
        ~AppTranslator() override;

        // Engine to retranslate after a language switch. May be null (tests).
        void setEngine(QQmlEngine* engine);

        QString language() const;
        QVariantList languages() const;
        bool rightToLeft() const;

        // Resolves `requested` against the system languages, loads the matching
        // files and installs them. Returns the effective code. Safe to call again.
        QString apply(const QString& requested);

        // Resource path of the app translation file for a code.
        static QString appTranslationPath(const QString& code);

        // Appends mf::fallbackFontFamilies() to the application font once.
        // Scripts the UI font lacks then render with the intended family.
        static void installFallbackFonts();

    signals:
        void languageChanged();

    private:
        bool loadAppTranslation(const QString& code);
        bool loadQtTranslation(const QString& code);

        QQmlEngine* m_engine = nullptr;
        QTranslator m_app;
        QTranslator m_qt;
        QString m_language;
        QString m_requested;
        bool m_applied = false;
};
