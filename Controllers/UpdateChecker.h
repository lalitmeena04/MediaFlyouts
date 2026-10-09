#pragma once

#include <QCryptographicHash>
#include <QFile>
#include <QObject>
#include <QString>

#include "InstallSource.h"
#include "Version.h"

class QJsonArray;
class QNetworkAccessManager;
class QNetworkReply;

// Checks the project's GitHub releases for a newer version than the running one.
// Installer builds download and run the new setup. Store and portable builds are
// pointed to the Store or the release page instead.
class UpdateChecker : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString status READ status NOTIFY changed)
        Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
        Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY changed)
        Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY changed)
        Q_PROPERTY(bool checking READ checking NOTIFY changed)
        Q_PROPERTY(QString releaseUrl READ releaseUrl NOTIFY changed)
        Q_PROPERTY(bool installerAvailable READ installerAvailable NOTIFY changed)
        Q_PROPERTY(QString installKind READ installKindId NOTIFY installKindChanged)
        Q_PROPERTY(QString installKindLabel READ installKindLabel NOTIFY installKindChanged)
        Q_PROPERTY(bool downloading READ downloading NOTIFY downloadChanged)
        Q_PROPERTY(qreal downloadProgress READ downloadProgress NOTIFY downloadChanged)
        Q_PROPERTY(bool downloaded READ downloaded NOTIFY downloadChanged)
        Q_PROPERTY(QString downloadError READ downloadError NOTIFY downloadChanged)
        Q_PROPERTY(QString downloadedFile READ downloadedFile NOTIFY downloadChanged)

    public:
        // Installer asset from the release API. Invalid when url is empty.
        struct InstallerAsset {
            QString name;
            QString url;
            qint64 size = -1;
            // Lower-case hex. Empty when the API gave no digest.
            QString sha256;
            bool valid() const { return !url.isEmpty(); }
        };

        explicit UpdateChecker(QObject* parent = nullptr);

        QString status() const;
        QString currentVersion() const;
        QString latestVersion() const;
        bool updateAvailable() const;
        bool checking() const;
        // Download page for the latest release. Falls back to the releases index.
        QString releaseUrl() const;
        // True when a newer release ships an installer and this copy may run it.
        bool installerAvailable() const;
        // Detected at construction. Tests override it.
        mf::InstallKind installKind() const;
        void setInstallKind(mf::InstallKind kind);
        QString installKindId() const;
        QString installKindLabel() const;
        // Only Inno Setup installs may replace themselves with a downloaded setup.
        static bool selfUpdateAllowed(mf::InstallKind kind);
        bool downloading() const;
        // 0..1 while downloading. 1 when done.
        qreal downloadProgress() const;
        bool downloaded() const;
        QString downloadError() const;
        QString downloadedFile() const;
        InstallerAsset installerAsset() const;

        static QString releasesApiUrl();
        static QString releasesPageUrl();

        // True if latest version is newer than current version.
        static bool isNewerVersion(const QString& current, const QString& latest);

        // Guard against following a link the release API did not vouch for.
        static bool isTrustedReleaseUrl(const QString& url);

        // Pick the setup executable from the release asset list.
        static InstallerAsset pickInstallerAsset(const QJsonArray& assets);
        // Plain file name of a setup executable. No path parts.
        static bool isInstallerName(const QString& name);
        // Hex from a "sha256:<hex>" digest. Empty for any other form.
        static QString sha256FromDigest(const QString& digest);
        // Downloaded bytes must match the digest. Size is the fallback check.
        static bool matchesAsset(const InstallerAsset& asset, qint64 size, const QString& sha256);

    public slots:
        void check();
        // Emit updateFound again for the last result. Opens the prompt again.
        void notifyUpdate();
        // Fetch the installer to the temp folder.
        void download();
        void cancelDownload();
        // Start the downloaded installer. Emits installStarted on success.
        void install();
        // Open the release page in the default browser.
        void openReleasePage();
        // Open the Microsoft Store updates page. Store installs update there.
        void openStorePage();

    signals:
        void changed();
        void downloadChanged();
        void installKindChanged();
        // A newer release was found by check().
        void updateFound(const QString& version);
        // Installer process is running. App should quit so files can be replaced.
        void installStarted();

    private:
        void finishDownload();
        void failDownload(const QString& message);
        void resetDownload();

        QNetworkAccessManager* m_net = nullptr;
        QNetworkReply* m_reply = nullptr;
        QFile m_file;
        QCryptographicHash m_hash{QCryptographicHash::Sha256};
        QString m_status = QStringLiteral("Not checked yet.");
        QString m_current = QStringLiteral(MEDIAFLYOUTS_VERSION);
        QString m_latest;
        QString m_releaseUrl;
        InstallerAsset m_asset;
        mf::InstallKind m_installKind = mf::InstallKind::Unknown;
        bool m_available = false;
        bool m_checking = false;
        bool m_downloading = false;
        bool m_downloaded = false;
        qreal m_progress = 0;
        QString m_downloadError;
        QString m_downloadedFile;
};
