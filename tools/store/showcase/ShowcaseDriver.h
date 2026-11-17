#pragma once

#include <QJsonObject>
#include <QLoggingCategory>
#include <QObject>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

#include <memory>

#include "PlaybackController.h"

class AppTranslator;
class NowPlaying;

Q_DECLARE_LOGGING_CATEGORY(lcShowcase)

struct ShowcaseOptions {
        QString stage;         // "parts" or "posters"
        QString workDir;       // grabbed views and generated icons
        QString outDir;        // finished posters, one folder per language
        QString previewDir;    // optional half-size JPEG copies
        QStringList only;      // poster names; empty selects all
        QStringList languages; // app language codes to render
        QJsonObject captions;  // see Captions.h
        int partScale = 6;     // device pixel ratio of the grabbed views
        QSize posterSize{2560, 1440};
};

// Bridge between the QML directors and the file system. Also feeds the real
// NowPlaying with demo tracks and switches the UI language.
class ShowcaseDriver : public QObject {
        Q_OBJECT
        Q_PROPERTY(int partScale READ partScale CONSTANT)
        Q_PROPERTY(int posterWidth READ posterWidth CONSTANT)
        Q_PROPERTY(int posterHeight READ posterHeight CONSTANT)
        Q_PROPERTY(QStringList languages READ languages CONSTANT)
        Q_PROPERTY(QString language READ language NOTIFY languageChanged)
        Q_PROPERTY(QString captionFont READ captionFont NOTIFY languageChanged)

    public:
        // translator may be null: the poster stage shows no app text.
        ShowcaseDriver(NowPlaying* nowPlaying, AppTranslator* translator, ShowcaseOptions options,
                       QObject* parent = nullptr);
        ~ShowcaseDriver() override;

        int partScale() const;
        int posterWidth() const;
        int posterHeight() const;
        QStringList languages() const;
        QString language() const;
        QString captionFont() const;
        int failures() const;
        int saved() const;

        // Later grabs, posters and captions belong to this language.
        Q_INVOKABLE void setLanguage(const QString& code);
        // App translation of a C++ string, for text the stand-in controllers supply.
        Q_INVOKABLE QString translate(const QString& context, const QString& text) const;
        // "headline" and "subtitle" of a poster in the current language.
        Q_INVOKABLE QVariantMap caption(const QString& poster);
        Q_INVOKABLE bool wanted(const QString& name) const;
        // Keys: title, artist, album, player, cover, position, duration (seconds),
        // playing, shuffle, repeat, sessions, session.
        Q_INVOKABLE void setTrack(const QVariantMap& track);
        Q_INVOKABLE void clearTrack();
        Q_INVOKABLE QUrl appIconUrl(int index);
        // Call right after creating a view. A window that is never shown gets
        // no resize event, so its content item would stay empty.
        Q_INVOKABLE bool prepare(QObject* window);
        Q_INVOKABLE bool grabPart(QObject* window, const QString& name);
        Q_INVOKABLE QUrl partUrl(const QString& name);
        Q_INVOKABLE bool savePoster(QObject* window, const QString& name);
        Q_INVOKABLE void fail(const QString& reason);
        Q_INVOKABLE void finish();

    signals:
        void languageChanged();

    private:
        class DemoPlayback;

        QString partPath(const QString& name) const;

        NowPlaying* m_nowPlaying = nullptr;
        AppTranslator* m_translator = nullptr;
        QString m_language;
        ShowcaseOptions m_options;
        std::unique_ptr<DemoPlayback> m_playback;
        int m_failures = 0;
        int m_saved = 0;
};
