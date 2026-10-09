#include "InstallSource.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>

#include <appmodel.h>
#endif

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
        case InstallKind::Store: return QStringLiteral("Microsoft Store");
        case InstallKind::Installer: return QStringLiteral("Installer");
        case InstallKind::Portable: return QStringLiteral("Portable");
        case InstallKind::Unknown: break;
    }
    return QStringLiteral("Unknown");
}

QString storeUpdatesUrl() {
    return QStringLiteral("ms-windows-store://downloadsandupdates");
}

} // namespace mf
