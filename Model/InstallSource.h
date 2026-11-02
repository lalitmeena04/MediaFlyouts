#pragma once

#include <QString>

namespace mf {

enum class InstallKind {
    Unknown,
    Store,
    Installer,
    Portable,
};

InstallKind classifyInstall(bool packaged, const QString& installDir, const QString& appDir);

// True inside an MSIX package. False on other platforms.
bool isPackagedProcess();

InstallKind detectInstallKind();

QString installKindId(InstallKind kind);
QString installKindLabel(InstallKind kind);

QString storeUpdatesUrl();

// Path as seen from outside the package container. Packaged apps get new files
// under AppData redirected to %LOCALAPPDATA%\Packages\<family>\LocalCache.
// Explorer runs outside the container and needs the real location.
QString packagedRedirectPath(const QString& path, const QString& roamingDir, const QString& localDir,
                             const QString& packageFamilyName);

// packagedRedirectPath() for this process. Unpackaged installs return the path as is.
QString hostVisiblePath(const QString& path);

} // namespace mf
