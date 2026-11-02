#include "InstallSource.h"

#include <string>

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>

#include <appmodel.h>
#endif

#include "Logging.h"

namespace mf {

namespace {

// Inno Setup AppId from packaging/MediaFlyouts.iss plus the "_is1" suffix.
constexpr char kUninstallKey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                                 "{B7E6F4B2-2C1A-4E9E-9A2E-6D0A1C3F5A10}_is1";

QString normalisedDir(const QString& path) {
    if(path.isEmpty())
        return {};
    QString clean = QDir::cleanPath(QDir::fromNativeSeparators(path));
    while(clean.size() > 1 && clean.endsWith(QLatin1Char('/')))
        clean.chop(1);
    return clean.toLower();
}

#ifdef Q_OS_WIN
bool runningPackaged() {
    UINT32 length = 0;
    const LONG rc = GetCurrentPackageFullName(&length, nullptr);
    return rc != APPMODEL_ERROR_NO_PACKAGE;
}

QString currentPackageFamilyName() {
    UINT32 length = 0;
    if(GetCurrentPackageFamilyName(&length, nullptr) != ERROR_INSUFFICIENT_BUFFER || length == 0)
        return {};
    std::wstring name(length, L'\0');
    if(GetCurrentPackageFamilyName(&length, name.data()) != ERROR_SUCCESS)
        return {};
    return QString::fromWCharArray(name.c_str());
}

QString innoInstallLocation() {
    const QStringList roots{QStringLiteral("HKEY_CURRENT_USER\\"), QStringLiteral("HKEY_LOCAL_MACHINE\\")};
    for(const QString& root : roots) {
        QSettings key(root + QString::fromLatin1(kUninstallKey), QSettings::NativeFormat);
        const QString location = key.value(QStringLiteral("InstallLocation")).toString();
        if(!location.isEmpty())
            return location;
    }
    return {};
}
#endif

} // namespace

InstallKind classifyInstall(bool packaged, const QString& installDir, const QString& appDir) {
    if(packaged)
        return InstallKind::Store;
    const QString install = normalisedDir(installDir);
    if(install.isEmpty())
        return InstallKind::Portable;
    const QString app = normalisedDir(appDir);
    // A ZIP copy can live next to an installed one. Only the installed folder may self-update.
    return app == install ? InstallKind::Installer : InstallKind::Portable;
}

bool isPackagedProcess() {
#ifdef Q_OS_WIN
    return runningPackaged();
#else
    return false;
#endif
}

InstallKind detectInstallKind() {
#ifdef Q_OS_WIN
    return classifyInstall(runningPackaged(), innoInstallLocation(), QCoreApplication::applicationDirPath());
#else
    return InstallKind::Unknown;
#endif
}

QString installKindId(InstallKind kind) {
    switch(kind) {
        case InstallKind::Store: return QStringLiteral("store");
        case InstallKind::Installer: return QStringLiteral("installer");
        case InstallKind::Portable: return QStringLiteral("portable");
        case InstallKind::Unknown: break;
    }
    return QStringLiteral("unknown");
}

QString installKindLabel(InstallKind kind) {
    switch(kind) {
        case InstallKind::Store: return QCoreApplication::translate("InstallSource", "Microsoft Store");
        case InstallKind::Installer: return QCoreApplication::translate("InstallSource", "Installer");
        case InstallKind::Portable: return QCoreApplication::translate("InstallSource", "Portable");
        case InstallKind::Unknown: break;
    }
    return QCoreApplication::translate("InstallSource", "Unknown");
}

QString storeUpdatesUrl() {
    return QStringLiteral("ms-windows-store://downloadsandupdates");
}

namespace {

// Sets rel to the part of path below root. False when path is not inside root.
bool relativeUnder(const QString& path, const QString& root, QString* rel) {
    if(root.isEmpty())
        return false;
    const QString cleanPath = QDir::cleanPath(QDir::fromNativeSeparators(path));
    const QString cleanRoot = normalisedDir(root);
    const QString lowerPath = cleanPath.toLower();
    if(lowerPath == cleanRoot) {
        rel->clear();
        return true;
    }
    if(!lowerPath.startsWith(cleanRoot + QLatin1Char('/')))
        return false;
    *rel = cleanPath.mid(cleanRoot.size() + 1);
    return true;
}

} // namespace

QString packagedRedirectPath(const QString& path, const QString& roamingDir, const QString& localDir,
                             const QString& packageFamilyName) {
    if(path.isEmpty() || packageFamilyName.isEmpty() || localDir.isEmpty())
        return path;
    const QString cache = QDir::cleanPath(QDir::fromNativeSeparators(localDir)) + QStringLiteral("/Packages/")
                          + packageFamilyName + QStringLiteral("/LocalCache/");

    QString rel;
    if(relativeUnder(path, roamingDir, &rel))
        return QDir::cleanPath(cache + QStringLiteral("Roaming/") + rel);
    if(!relativeUnder(path, localDir, &rel))
        return path;
    // Already inside Packages: that is a real location, not a virtual one.
    const QString first = rel.section(QLatin1Char('/'), 0, 0);
    if(first.compare(QStringLiteral("Packages"), Qt::CaseInsensitive) == 0)
        return path;
    return QDir::cleanPath(cache + QStringLiteral("Local/") + rel);
}

QString hostVisiblePath(const QString& path) {
#ifdef Q_OS_WIN
    if(!runningPackaged())
        return path;
    const QString family = currentPackageFamilyName();
    // The container keeps the real folders in these variables.
    const QString roaming = qEnvironmentVariable("APPDATA");
    const QString local = qEnvironmentVariable("LOCALAPPDATA");
    const QString mapped = packagedRedirectPath(path, roaming, local, family);
    if(mapped != path)
        TRC_DEBUG("packaged path: {} -> {}", path.toStdString(), mapped.toStdString());
    return mapped;
#else
    return path;
#endif
}

} // namespace mf
