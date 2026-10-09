#pragma once

#include <QString>

namespace mf {

// How this copy of the app reached the PC. Decides how updates are delivered.
enum class InstallKind {
    Unknown,
    // MSIX package. The Microsoft Store updates it.
    Store,
    // Inno Setup install. The app can download and run the new setup itself.
    Installer,
    // Unpacked ZIP. The user swaps the folder by hand.
    Portable,
};

// Pure decision. `appDir` and `installDir` are compared after normalisation.
// Empty installDir means no uninstall entry was found.
InstallKind classifyInstall(bool packaged, const QString& installDir, const QString& appDir);

// Probe the running process. Windows only. Unknown elsewhere.
InstallKind detectInstallKind();

// Stable id for QML and logs: "store", "installer", "portable", "unknown".
QString installKindId(InstallKind kind);
// Text for the UI: "Microsoft Store", "Installer", "Portable", "Unknown".
QString installKindLabel(InstallKind kind);

// Store page that lists pending app updates.
QString storeUpdatesUrl();

} // namespace mf
