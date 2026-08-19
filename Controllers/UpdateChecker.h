#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;

// Checks the project's GitHub releases for a newer version than the running one.
class UpdateChecker : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY changed)
        Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
        Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY changed)
        Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY changed)
        Q_PROPERTY(bool checking READ checking NOTIFY changed)

    public:
        explicit UpdateChecker(QObject* parent = nullptr);

        QString status() const;
        QString currentVersion() const;
        QString latestVersion() const;
        bool updateAvailable() const;
        bool checking() const;

        // true when `latest` is a strictly newer dotted-numeric version than `current`
        static bool isNewerVersion(const QString& current, const QString& latest);

    public slots:
        void check();

    signals:
        void changed();

    private:
        QNetworkAccessManager* m_net = nullptr;
        QString m_status = QStringLiteral("Not checked yet.");
        QString m_current = QStringLiteral("0.1.0");
        QString m_latest;
        bool m_available = false;
        bool m_checking = false;
};
