#pragma once

#include <QObject>
#include <QString>

class Settings;

// The `whatsNew` QML context property. Shows the bundled notes once per version.
class WhatsNewController : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool pending READ pending NOTIFY changed)
        Q_PROPERTY(QString version READ version CONSTANT)
        Q_PROPERTY(QString text READ text CONSTANT)
        Q_PROPERTY(QString body READ body CONSTANT)

    public:
        explicit WhatsNewController(Settings* settings, QObject* parent = nullptr);
        // Test hook. Default is the compiled version.
        WhatsNewController(Settings* settings, const QString& currentVersion, QObject* parent = nullptr);

        // Reads lastSeenVersion. A first run records the version and stays quiet.
        void start();

        bool pending() const;
        QString version() const;
        QString text() const;
        QString body() const;
        QString currentVersion() const;

        Q_INVOKABLE void markSeen();
        Q_INVOKABLE void requestShow();

    signals:
        void changed();
        void showRequested();

    private:
        Settings* m_settings;
        QString m_current;
        QString m_text;
        bool m_pending = false;
};
