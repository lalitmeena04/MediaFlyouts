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
        struct InstallerAsset {
            QString name;
            QString url;
            qint64 size = -1;
            QString sha256;
            bool valid() const { return !url.isEmpty(); }
        };

        explicit UpdateChecker(QObject* parent = nullptr);

        QString status() const;
        QString currentVersion() const;
        QString latestVersion() const;
        bool updateAvailable() const;
        bool checking() const;
        QString releaseUrl() const;
        bool installerAvailable() const;
        mf::InstallKind installKind() const;
        void setInstallKind(mf::InstallKind kind);
        QString installKindId() const;
        QString installKindLabel() const;
        static bool selfUpdateAllowed(mf::InstallKind kind);
        bool downloading() const;
        qreal downloadProgress() const;
        bool downloaded() const;
        QString downloadError() const;
        QString downloadedFile() const;
        InstallerAsset installerAsset() const;

        static QString releasesApiUrl();
        static QString releasesPageUrl();

        static bool isNewerVersion(const QString& current, const QString& latest);

        static bool isTrustedReleaseUrl(const QString& url);

        static InstallerAsset pickInstallerAsset(const QJsonArray& assets);
        static bool isInstallerName(const QString& name);
        static QString sha256FromDigest(const QString& digest);
        static bool matchesAsset(const InstallerAsset& asset, qint64 size, const QString& sha256);
        static QString downloadDir();
        static QString fileSha256(const QString& path);
        static bool isDownloadedInstaller(const QString& path, const InstallerAsset& asset, const QString& dir);
        static bool launchElevated(const QString& path);

    public slots:
        void check();
        void notifyUpdate();
        void download();
        void cancelDownload();
        void install();
        void openReleasePage();
        void openStorePage();

    signals:
        void changed();
        void downloadChanged();
        void installKindChanged();
        void updateFound(const QString& version);
        void installStarted();

    private:
        void finishDownload();
        void failDownload(const QString& message);
        void resetDownload();
        bool storeChunk(const QByteArray& chunk);

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
        bool m_writeFailed = false;
        qreal m_progress = 0;
        QString m_downloadError;
        QString m_downloadedFile;
};
