#include "UpdateChecker.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

#include "Logging.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
constexpr char kReleasesApi[] = "https://api.github.com/repos/scleaverzer0ne/MediaFlyouts-Releases/releases/latest";
constexpr char kReleasesPage[] = "https://github.com/scleaverzer0ne/MediaFlyouts-Releases/releases/latest";
constexpr char kUserAgent[] = "MediaFlyouts";
} // namespace

UpdateChecker::UpdateChecker(QObject* parent)
    : QObject(parent), m_net(new QNetworkAccessManager(this)), m_installKind(mf::detectInstallKind()) {
    TRC_INFO("update: install kind = {}", mf::installKindId(m_installKind).toStdString());
}

bool UpdateChecker::isNewerVersion(const QString& current, const QString& latest) {
    // Compare numeric core only. Dev builds have build-number suffix.
    const QStringList pa = QString(current).remove('v').section('-', 0, 0).split('.');
    const QStringList pb = QString(latest).remove('v').section('-', 0, 0).split('.');
    const int n = qMax(pa.size(), pb.size());
    for(int i = 0; i < n; ++i) {
        const int va = i < pa.size() ? pa[i].toInt() : 0;
        const int vb = i < pb.size() ? pb[i].toInt() : 0;
        if(vb != va)
            return vb > va;
    }
    return false;
}

QString UpdateChecker::status() const {
    return m_status;
}
QString UpdateChecker::currentVersion() const {
    return m_current;
}
QString UpdateChecker::latestVersion() const {
    return m_latest;
}
bool UpdateChecker::updateAvailable() const {
    return m_available;
}
bool UpdateChecker::checking() const {
    return m_checking;
}

QString UpdateChecker::releaseUrl() const {
    return isTrustedReleaseUrl(m_releaseUrl) ? m_releaseUrl : releasesPageUrl();
}
bool UpdateChecker::installerAvailable() const {
    return m_available && m_asset.valid() && selfUpdateAllowed(m_installKind);
}
mf::InstallKind UpdateChecker::installKind() const {
    return m_installKind;
}
void UpdateChecker::setInstallKind(mf::InstallKind kind) {
    if(m_installKind == kind)
        return;
    m_installKind = kind;
    emit installKindChanged();
    emit changed();
}
QString UpdateChecker::installKindId() const {
    return mf::installKindId(m_installKind);
}
QString UpdateChecker::installKindLabel() const {
    return mf::installKindLabel(m_installKind);
}
bool UpdateChecker::selfUpdateAllowed(mf::InstallKind kind) {
    return kind == mf::InstallKind::Installer;
}
bool UpdateChecker::downloading() const {
    return m_downloading;
}
qreal UpdateChecker::downloadProgress() const {
    return m_progress;
}
bool UpdateChecker::downloaded() const {
    return m_downloaded;
}
QString UpdateChecker::downloadError() const {
    return m_downloadError;
}
QString UpdateChecker::downloadedFile() const {
    return m_downloadedFile;
}
UpdateChecker::InstallerAsset UpdateChecker::installerAsset() const {
    return m_asset;
}

QString UpdateChecker::releasesApiUrl() {
    return QString::fromLatin1(kReleasesApi);
}

QString UpdateChecker::releasesPageUrl() {
    return QString::fromLatin1(kReleasesPage);
}

bool UpdateChecker::isTrustedReleaseUrl(const QString& url) {
    const QUrl parsed(url);
    return parsed.scheme() == QLatin1String("https") && parsed.host() == QLatin1String("github.com");
}

bool UpdateChecker::isInstallerName(const QString& name) {
    static const QRegularExpression pattern(QStringLiteral(R"(^[A-Za-z0-9._-]+-setup\.exe$)"),
                                            QRegularExpression::CaseInsensitiveOption);
    return pattern.match(name).hasMatch();
}

QString UpdateChecker::sha256FromDigest(const QString& digest) {
    const QLatin1String prefix("sha256:");
    if(!digest.startsWith(prefix, Qt::CaseInsensitive))
        return {};
    const QString hex = digest.mid(prefix.size()).trimmed().toLower();
    static const QRegularExpression hex64(QStringLiteral("^[0-9a-f]{64}$"));
    return hex64.match(hex).hasMatch() ? hex : QString();
}

UpdateChecker::InstallerAsset UpdateChecker::pickInstallerAsset(const QJsonArray& assets) {
    for(const QJsonValue& value : assets) {
        const QJsonObject obj = value.toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        const QString url = obj.value(QStringLiteral("browser_download_url")).toString();
        if(!isInstallerName(name) || !isTrustedReleaseUrl(url))
            continue;
        InstallerAsset asset;
        asset.name = name;
        asset.url = url;
        asset.size = obj.value(QStringLiteral("size")).toInteger(-1);
        asset.sha256 = sha256FromDigest(obj.value(QStringLiteral("digest")).toString());
        return asset;
    }
    return {};
}

bool UpdateChecker::matchesAsset(const InstallerAsset& asset, qint64 size, const QString& sha256) {
    if(size <= 0)
        return false;
    if(asset.size >= 0 && size != asset.size)
        return false;
    if(!asset.sha256.isEmpty())
        return sha256.compare(asset.sha256, Qt::CaseInsensitive) == 0;
    return true;
}

QString UpdateChecker::downloadDir() {
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

QString UpdateChecker::fileSha256(const QString& path) {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
        return {};
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if(!hash.addData(&file))
        return {};
    return QString::fromLatin1(hash.result().toHex());
}

bool UpdateChecker::isDownloadedInstaller(const QString& path, const InstallerAsset& asset, const QString& dir) {
    if(path.isEmpty() || !asset.valid() || !isInstallerName(asset.name))
        return false;
    const QFileInfo info(path);
    if(!info.isFile() || info.isSymLink() || info.fileName() != asset.name)
        return false;
    // Canonical paths resolve links and casing. The folder must be ours.
    const QString haveDir = QFileInfo(info.absolutePath()).canonicalFilePath();
    const QString wantDir = QFileInfo(dir).canonicalFilePath();
    if(haveDir.isEmpty() || haveDir != wantDir)
        return false;
    // Hash again. The file may have been swapped since the download.
    const QString sha256 = asset.sha256.isEmpty() ? QString() : fileSha256(path);
    return matchesAsset(asset, info.size(), sha256);
}

bool UpdateChecker::launchElevated(const QString& path) {
    const QFileInfo target(path);
    if(path.isEmpty() || !target.isAbsolute() || !target.isFile() || target.isSymLink())
        return false;
#ifdef Q_OS_WIN
    const std::wstring file = QDir::toNativeSeparators(path).toStdWString();
    SHELLEXECUTEINFOW info{};
    info.cbSize = sizeof(info);
    // NO_UI hides shell error boxes. The UAC prompt still shows.
    info.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
    info.lpVerb = L"runas";
    info.lpFile = file.c_str();
    info.nShow = SW_SHOWNORMAL;
    if(ShellExecuteExW(&info))
        return true;
    TRC_WARN("update: ShellExecuteEx failed: {}", GetLastError());
    return false;
#else
    return false;
#endif
}

void UpdateChecker::openReleasePage() {
    QDesktopServices::openUrl(QUrl(releaseUrl()));
}

void UpdateChecker::openStorePage() {
    QDesktopServices::openUrl(QUrl(mf::storeUpdatesUrl()));
}

void UpdateChecker::notifyUpdate() {
    if(m_available)
        emit updateFound(m_latest);
}

void UpdateChecker::resetDownload() {
    m_downloading = false;
    m_downloaded = false;
    m_writeFailed = false;
    m_progress = 0;
    m_downloadError.clear();
    m_downloadedFile.clear();
}

void UpdateChecker::failDownload(const QString& message) {
    TRC_WARN("update: download failed: {}", message.toStdString());
    if(m_file.isOpen())
        m_file.close();
    if(!m_file.fileName().isEmpty())
        m_file.remove();
    resetDownload();
    m_downloadError = message;
    emit downloadChanged();
}

void UpdateChecker::download() {
    if(m_downloading || !installerAvailable())
        return;

    const QString dir = downloadDir();
    if(dir.isEmpty()) {
        failDownload(QStringLiteral("Couldn't find a folder for the download."));
        return;
    }
    m_file.setFileName(QDir(dir).filePath(m_asset.name));
    if(!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        failDownload(QStringLiteral("Couldn't create the download file."));
        return;
    }
    m_hash.reset();
    resetDownload();
    m_downloading = true;
    emit downloadChanged();
    TRC_INFO("update: downloading {} -> {}", m_asset.name.toStdString(), m_file.fileName().toStdString());

    QNetworkRequest request{QUrl(m_asset.url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
    m_reply = m_net->get(request);
    connect(m_reply, &QNetworkReply::readyRead, this, [this] {
        // Abort ends in finishDownload, which reports the write error.
        if(!storeChunk(m_reply->readAll()))
            m_reply->abort();
    });
    connect(m_reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        const qint64 size = total > 0 ? total : m_asset.size;
        m_progress = size > 0 ? qBound<qreal>(0, qreal(received) / qreal(size), 1) : 0;
        emit downloadChanged();
    });
    connect(m_reply, &QNetworkReply::finished, this, &UpdateChecker::finishDownload);
}

bool UpdateChecker::storeChunk(const QByteArray& chunk) {
    if(m_writeFailed || chunk.isEmpty())
        return !m_writeFailed;
    m_hash.addData(chunk);
    if(m_file.write(chunk) == chunk.size())
        return true;
    TRC_WARN("update: write failed: {}", m_file.errorString().toStdString());
    m_writeFailed = true;
    return false;
}

void UpdateChecker::finishDownload() {
    QNetworkReply* reply = m_reply;
    m_reply = nullptr;
    reply->deleteLater();

    storeChunk(reply->readAll());
    const bool flushed = m_file.flush();
    m_file.close();

    if(m_writeFailed || !flushed) {
        failDownload(QStringLiteral("Couldn't write the download file."));
        return;
    }
    if(reply->error() == QNetworkReply::OperationCanceledError) {
        TRC_INFO("update: download cancelled");
        m_file.remove();
        resetDownload();
        emit downloadChanged();
        return;
    }
    if(reply->error() != QNetworkReply::NoError) {
        TRC_WARN("update: network error: {}", reply->errorString().toStdString());
        failDownload(QStringLiteral("Download failed. Check your connection and try again."));
        return;
    }
    const QString sha256 = QString::fromLatin1(m_hash.result().toHex());
    if(!matchesAsset(m_asset, m_file.size(), sha256)) {
        failDownload(QStringLiteral("The download didn't match the release. Try again."));
        return;
    }

    m_downloading = false;
    m_downloaded = true;
    m_progress = 1;
    m_downloadedFile = m_file.fileName();
    TRC_INFO("update: download verified, {} bytes", m_file.size());
    emit downloadChanged();
}

void UpdateChecker::cancelDownload() {
    if(m_reply)
        m_reply->abort();
}

void UpdateChecker::install() {
    if(!m_downloaded || m_downloadedFile.isEmpty())
        return;
    // The file can vanish between download and install. Never run anything else.
    if(!isDownloadedInstaller(m_downloadedFile, m_asset, downloadDir())) {
        failDownload(QStringLiteral("The downloaded installer is missing or changed. Download it again."));
        return;
    }
    // The setup installs for all users, so it needs elevation.
    if(!launchElevated(m_downloadedFile)) {
        TRC_WARN("update: installer failed to start: {}", m_downloadedFile.toStdString());
        m_downloadError = QStringLiteral("Couldn't start the installer. Allow it when Windows asks for permission.");
        emit downloadChanged();
        return;
    }
    TRC_INFO("update: installer started, quitting");
    emit installStarted();
}

void UpdateChecker::check() {
    if(m_checking)
        return;
    m_checking = true;
    m_status = QStringLiteral("Checking for updates…");
    emit changed();

    QNetworkRequest request{QUrl(releasesApiUrl())};
    request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
    QNetworkReply* reply = m_net->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        m_checking = false;

        if(reply->error() != QNetworkReply::NoError) {
            TRC_WARN("update: check failed: {}", reply->errorString().toStdString());
            m_status = QStringLiteral("Couldn't check for updates right now.");
            emit changed();
            return;
        }

        const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        const QString tag = obj.value(QStringLiteral("tag_name")).toString();
        if(tag.isEmpty()) {
            TRC_WARN("update: release response had no tag_name");
            m_status = QStringLiteral("No releases found.");
            emit changed();
            return;
        }

        m_latest = tag;
        // Only follow a GitHub link. Anything else falls back to the releases index.
        m_releaseUrl = obj.value(QStringLiteral("html_url")).toString();
        m_available = isNewerVersion(m_current, tag);
        m_status = m_available ? QStringLiteral("Update available: %1").arg(tag)
                               : QStringLiteral("You're on the latest version.");
        if(!m_downloading) {
            m_asset = pickInstallerAsset(obj.value(QStringLiteral("assets")).toArray());
            resetDownload();
            emit downloadChanged();
        }
        TRC_INFO("update: latest={} current={} available={} installer={}", tag.toStdString(), m_current.toStdString(),
                 m_available, m_asset.valid());
        emit changed();
        if(m_available)
            emit updateFound(tag);
    });
}
