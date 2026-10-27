#pragma once

#include <QIcon>
#include <QString>

#include "Logging.h"

namespace mf {

// Multi-size app icon. Same file the exe and installer embed.
// Compiled in by qt_add_resources(appicon) in CMakeLists.txt.
inline const QString kAppIconResource = QStringLiteral(":/icons/MediaFlyouts.ico");

// Logo bitmap for QML headers. Path form that Image.source accepts.
inline const QString kAppLogoUrl = QStringLiteral("qrc:/icons/icon-256.png");

// Load the app icon from a resource or file path. Null icon when the path
// has no readable image. Caller keeps one QIcon for tray and windows.
inline QIcon loadAppIcon(const QString& path = kAppIconResource) {
    QIcon icon(path);
    if(icon.isNull())
        TRC_WARN("app icon: no image at {}", path.toStdString());
    else
        TRC_DEBUG("app icon: loaded {} sizes from {}", icon.availableSizes().size(), path.toStdString());
    return icon;
}

} // namespace mf
