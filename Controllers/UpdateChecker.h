#pragma once

#include <QObject>
#include <QString>

#include "Version.h"

class QNetworkAccessManager;

// Checks the project's GitHub releases for a newer version than the running one.
class UpdateChecker : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY changed)
        Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
        Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY changed)
        Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY changed)
        Q_PROPERTY(bool checking READ checking NOTIFY changed)
        Q_PROPERTY(QString releaseUrl READ releaseUrl NOTIFY changed)

    public:
        explicit UpdateChecker(QObject* parent = nullptr);

        QString status() const;
        QString currentVersion() const;
        QString latestVersion() const;
        bool updateAvailable() const;
        bool checking() const;
        // Download page for the latest release. Falls back to the releases index.
        QString releaseUrl() const;

        static QString releasesApiUrl();
        static QString releasesPageUrl();

        // True if latest version is newer than current version.
        static bool isNewerVersion(const QString& current, const QString& latest);

        // Guard against following a link the release API did not vouch for.
        static bool isTrustedReleaseUrl(const QString& url);

    public slots:
        void check();
        // Open the release page in the default browser.
        void openReleasePage();

    signals:
        void changed();

    private:
        QNetworkAccessManager* m_net = nullptr;
        QString m_status = QStringLiteral("Not checked yet.");
        QString m_current = QStringLiteral(MEDIAFLYOUTS_VERSION);
        QString m_latest;
        QString m_releaseUrl;
        bool m_available = false;
        bool m_checking = false;
};
