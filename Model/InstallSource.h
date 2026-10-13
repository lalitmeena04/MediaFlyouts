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

InstallKind detectInstallKind();

QString installKindId(InstallKind kind);
QString installKindLabel(InstallKind kind);

QString storeUpdatesUrl();

} // namespace mf
