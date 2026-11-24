#include "BrowserIntegration.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

#include "Logging.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
const QString kDevelopmentId = QStringLiteral("pmblljpkoajpojjjeidepiddahgpilge");
const QString kFirefoxId = QStringLiteral("browser-artwork@mediaflyouts.local");
const QString kHostName = QStringLiteral("com.mediaflyouts.browser_artwork");

QString storeId(const QString& browser) {
    if(browser == QStringLiteral("chrome") || browser == QStringLiteral("brave"))
        return QStringLiteral(MEDIAFLYOUTS_CHROME_EXTENSION_ID);
    if(browser == QStringLiteral("msedge"))
        return QStringLiteral(MEDIAFLYOUTS_EDGE_EXTENSION_ID);
    return {};
}

bool validChromiumId(const QString& id) {
    static const QRegularExpression pattern(QStringLiteral("^[a-p]{32}$"));
    return pattern.match(id).hasMatch();
}

bool isChromiumBrowser(const QString& browser) {
    return browser == QStringLiteral("chrome") || browser == QStringLiteral("msedge") ||
           browser == QStringLiteral("brave");
}

bool copyDirectoryContents(const QString& sourcePath, const QString& destinationPath) {
    QDir source(sourcePath);
    if(!source.exists()) {
        TRC_WARN("browser extension setup: source folder is missing: {}", sourcePath.toStdString());
        return false;
    }

    QDir destination(destinationPath);
    if(!destination.exists() && !destination.mkpath(destination.absolutePath())) {
        TRC_WARN("browser extension setup: could not create folder: {}", destination.absolutePath().toStdString());
        return false;
    }

    const QFileInfoList entries =
        source.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for(const QFileInfo& entry : entries) {
        const QString target = destination.filePath(entry.fileName());
        if(entry.isDir()) {
            if(!copyDirectoryContents(entry.absoluteFilePath(), target)) {
                TRC_WARN("browser extension setup: could not copy folder: {}", entry.absoluteFilePath().toStdString());
                return false;
            }
        } else {
            QFile::remove(target);
            if(!QFile::copy(entry.absoluteFilePath(), target)) {
                TRC_WARN("browser extension setup: could not copy file: {}", entry.absoluteFilePath().toStdString());
                return false;
            }
        }
    }
    return true;
}

bool openBrowserUrl(const QString& browser, const QUrl& url) {
#ifdef Q_OS_WIN
    const QString executable = browser + QStringLiteral(".exe");
    const QString parameters = url.toString(QUrl::FullyEncoded);
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(nullptr, L"open", reinterpret_cast<LPCWSTR>(executable.utf16()),
                      reinterpret_cast<LPCWSTR>(parameters.utf16()), nullptr, SW_SHOWNORMAL));
    return result > 32;
#else
    Q_UNUSED(browser);
    return QDesktopServices::openUrl(url);
#endif
}
}

BrowserIntegration::BrowserIntegration(QObject* parent) : QObject(parent) {}

QString BrowserIntegration::status() const {
    return m_status;
}

bool BrowserIntegration::developmentAvailable() const {
#ifdef QT_NO_DEBUG
    return false;
#else
    return true;
#endif
}

bool BrowserIntegration::manualSetupAvailable(const QString& browser) const {
    return isChromiumBrowser(browser);
}

bool BrowserIntegration::canInstall(const QString& browser) const {
    return !storeUrl(browser, storeId(browser), QStringLiteral(MEDIAFLYOUTS_FIREFOX_EXTENSION_URL)).isEmpty();
}

QUrl BrowserIntegration::storeUrl(const QString& browser, const QString& chromiumId, const QString& firefoxUrl) {
    if(browser == QStringLiteral("firefox")) {
        const QUrl url(firefoxUrl);
        return url.scheme() == QStringLiteral("https") && url.host() == QStringLiteral("addons.mozilla.org")
                   ? url : QUrl();
    }
    if(!validChromiumId(chromiumId))
        return {};
    if(browser == QStringLiteral("chrome") || browser == QStringLiteral("brave"))
        return QUrl(QStringLiteral("https://chromewebstore.google.com/detail/%1").arg(chromiumId));
    if(browser == QStringLiteral("msedge"))
        return QUrl(QStringLiteral("https://microsoftedge.microsoft.com/addons/detail/%1").arg(chromiumId));
    return {};
}

QString BrowserIntegration::externalExtensionRegistryPath(const QString& browser, const QString& chromiumId) {
    if(!validChromiumId(chromiumId))
        return {};
    QString vendor;
    if(browser == QStringLiteral("chrome"))
        vendor = QStringLiteral("Google\\Chrome");
    else if(browser == QStringLiteral("msedge"))
        vendor = QStringLiteral("Microsoft\\Edge");
    else if(browser == QStringLiteral("brave"))
        vendor = QStringLiteral("BraveSoftware\\Brave");
    else
        return {};
    return QStringLiteral("Software\\%1\\Extensions\\%2").arg(vendor, chromiumId);
}

QUrl BrowserIntegration::externalExtensionUpdateUrl(const QString& browser) {
    if(browser == QStringLiteral("chrome") || browser == QStringLiteral("brave"))
        return QUrl(QStringLiteral("https://clients2.google.com/service/update2/crx"));
    if(browser == QStringLiteral("msedge"))
        return QUrl(QStringLiteral("https://edge.microsoft.com/extensionwebstorebase/v1/crx"));
    return {};
}

QString BrowserIntegration::nativeHostRegistryPath(const QString& browser) {
    QString vendor;
    if(browser == QStringLiteral("chrome"))
        vendor = QStringLiteral("Google\\Chrome");
    else if(browser == QStringLiteral("brave"))
        vendor = QStringLiteral("BraveSoftware\\Brave-Browser");
    else if(browser == QStringLiteral("msedge"))
        vendor = QStringLiteral("Microsoft\\Edge");
    else if(browser == QStringLiteral("firefox"))
        vendor = QStringLiteral("Mozilla");
    else
        return {};
    return QStringLiteral("Software\\%1\\NativeMessagingHosts\\%2").arg(vendor, kHostName);
}

QString BrowserIntegration::prepareExtensionSetupFolder(const QString& browser) {
    const bool firefox = browser == QStringLiteral("firefox");
    if(!firefox && !isChromiumBrowser(browser))
        return {};

    const QString kind = firefox ? QStringLiteral("firefox") : QStringLiteral("chromium");
    const QString source = QDir(QCoreApplication::applicationDirPath())
                               .filePath(QStringLiteral("browser-extensions/%1").arg(kind));
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if(base.isEmpty())
        return {};

    const QString destination = QDir(base).filePath(QStringLiteral("browser-extensions/%1").arg(kind));
    QDir(destination).removeRecursively();
    if(!QDir().mkpath(destination) || !copyDirectoryContents(source, destination))
        return {};
    return destination;
}

QJsonObject BrowserIntegration::nativeManifest(const QString& browser, const QString& hostPath,
                                               const QString& chromiumId) {
    if(hostPath.isEmpty())
        return {};
    QJsonObject manifest{{QStringLiteral("name"), kHostName},
                         {QStringLiteral("description"), QStringLiteral("MediaFlyouts browser artwork bridge")},
                         {QStringLiteral("path"), QDir::toNativeSeparators(hostPath)},
                         {QStringLiteral("type"), QStringLiteral("stdio")}};
    if(browser == QStringLiteral("firefox"))
        manifest.insert(QStringLiteral("allowed_extensions"), QJsonArray{kFirefoxId});
    else if((browser == QStringLiteral("chrome") || browser == QStringLiteral("msedge") ||
             browser == QStringLiteral("brave")) &&
            validChromiumId(chromiumId))
        manifest.insert(QStringLiteral("allowed_origins"),
                        QJsonArray{QStringLiteral("chrome-extension://%1/").arg(chromiumId)});
    else
        return {};
    return manifest;
}

bool BrowserIntegration::writeManifest(const QString& manifestPath, const QJsonObject& manifest) {
    if(manifest.isEmpty() || !QDir().mkpath(QFileInfo(manifestPath).absolutePath()))
        return false;
    QSaveFile file(manifestPath);
    if(!file.open(QIODevice::WriteOnly))
        return false;
    const QByteArray bytes = QJsonDocument(manifest).toJson();
    return file.write(bytes) == bytes.size() && file.commit();
}

bool BrowserIntegration::registerHost(const QString& browser, const QString& chromiumId) {
#ifdef Q_OS_WIN
    const QString hostPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("MediaFlyoutsBrowserHost.exe"));
    if(!QFileInfo::exists(hostPath)) {
        TRC_WARN("browser integration: native host is missing");
        return false;
    }
    const QJsonObject manifest = nativeManifest(browser, hostPath, chromiumId);
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString manifestPath = QDir(folder).filePath(QStringLiteral("browser-integration/%1.json").arg(browser));
    if(folder.isEmpty() || !writeManifest(manifestPath, manifest)) {
        TRC_WARN("browser integration: could not write native host manifest");
        return false;
    }
    const QString registryPath = nativeHostRegistryPath(browser);
    if(registryPath.isEmpty())
        return false;
    QSettings registry(QStringLiteral("HKEY_CURRENT_USER\\%1").arg(registryPath), QSettings::NativeFormat);
    registry.setValue(QStringLiteral("Default"), QDir::toNativeSeparators(manifestPath));
    registry.sync();
    if(registry.status() != QSettings::NoError) {
        TRC_WARN("browser integration: could not register native host for {}", browser.toStdString());
        return false;
    }
    TRC_INFO("browser integration: native host registered for {}", browser.toStdString());
    return true;
#else
    Q_UNUSED(browser);
    Q_UNUSED(chromiumId);
    return false;
#endif
}

bool BrowserIntegration::registerExternalExtension(const QString& browser, const QString& chromiumId) {
#ifdef Q_OS_WIN
    const QString key = externalExtensionRegistryPath(browser, chromiumId);
    const QUrl updateUrl = externalExtensionUpdateUrl(browser);
    if(key.isEmpty() || updateUrl.isEmpty())
        return false;
    QSettings registry(QStringLiteral("HKEY_LOCAL_MACHINE\\%1").arg(key), QSettings::Registry32Format);
    registry.setValue(QStringLiteral("update_url"), updateUrl.toString());
    registry.sync();
    if(registry.status() != QSettings::NoError) {
        TRC_WARN("browser integration: could not register external extension for {}", browser.toStdString());
        return false;
    }
    TRC_INFO("browser integration: external extension registered for {}", browser.toStdString());
    return true;
#else
    Q_UNUSED(browser);
    Q_UNUSED(chromiumId);
    return false;
#endif
}

void BrowserIntegration::install(const QString& browser) {
    if(!canInstall(browser)) {
        setStatus(tr("The extension is not available in this browser's store yet."));
        return;
    }
    if(!registerHost(browser, storeId(browser))) {
        setStatus(tr("Could not set up the browser connection."));
        return;
    }
    const bool externallyRegistered = browser == QStringLiteral("firefox") ||
                                      registerExternalExtension(browser, storeId(browser));
    const QUrl url = storeUrl(browser, storeId(browser), QStringLiteral(MEDIAFLYOUTS_FIREFOX_EXTENSION_URL));
    if(!openBrowserUrl(browser, url)) {
        TRC_WARN("browser integration: could not open store for {}", browser.toStdString());
        setStatus(tr("Could not open the extension store."));
        return;
    }
    if(browser != QStringLiteral("firefox") && !externallyRegistered)
        TRC_WARN("browser integration: external registration unavailable; falling back to store");
    setStatus(tr("Confirm the installation in your browser."));
}

void BrowserIntegration::openExtensionSetup(const QString& browser) {
    const bool firefox = browser == QStringLiteral("firefox");
    const bool chromium = isChromiumBrowser(browser);
#ifdef QT_NO_DEBUG
    if(!chromium) {
        TRC_WARN("browser integration: manual extension setup is not available for {}", browser.toStdString());
        setStatus(tr("The extension is not available in this browser's store yet."));
        return;
    }
#else
    if(!firefox && !chromium) {
        TRC_WARN("browser integration: unsupported browser setup requested for {}", browser.toStdString());
        setStatus(tr("Could not set up the development extension."));
        return;
    }
#endif

    if(!developmentAvailable() && canInstall(browser)) {
        setStatus(tr("Confirm the installation in your browser."));
        return;
    }

    const QString folder = prepareExtensionSetupFolder(browser);
    const QString extensionId = firefox ? QString() : kDevelopmentId;
    if(!QFileInfo::exists(QDir(folder).filePath(QStringLiteral("manifest.json"))) ||
       !registerHost(browser, extensionId)) {
        setStatus(tr("Could not set up the development extension."));
        return;
    }
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(folder))) {
        TRC_WARN("browser integration: could not open package folder for {}", browser.toStdString());
        setStatus(tr("Could not open the extension folder."));
        return;
    }
    if(chromium) {
        const QUrl extensionsUrl(QStringLiteral("%1://extensions").arg(browser));
        if(!openBrowserUrl(browser, extensionsUrl)) {
            TRC_WARN("browser integration: could not open extension settings for {}", browser.toStdString());
            setStatus(tr("Could not open the extension store."));
            return;
        }
    }
    setStatus(tr("Load the extension from this folder using your browser's developer tools."));
}

void BrowserIntegration::setStatus(const QString& status) {
    if(m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}