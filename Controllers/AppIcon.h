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

// Pro icon styles a licensed user can pick. Index = Settings::proIconStyle.
// Views/AppLogo.qml holds the same logo list; QmlContractTest keeps them equal.
struct ProIconStyle {
        const char* id;
        QString icon;
        QString logo;
};
inline const ProIconStyle kProIconStyles[] = {
    {"bar", QStringLiteral(":/icons/MediaFlyoutsProBar.ico"), QStringLiteral("qrc:/icons/icon-pro-bar-256.png")},
    {"ribbon", QStringLiteral(":/icons/MediaFlyoutsProRibbon.ico"),
     QStringLiteral("qrc:/icons/icon-pro-ribbon-256.png")},
};
inline constexpr int kProIconStyleCount = 2;

// Icon resource for the current licence and style. The Pro mark is only for
// users who hold a Pro licence, never for the always-unlocked installer
// build. An out-of-range style falls back to the default style.
inline QString appIconResource(bool proLicensed, int proStyle = 0) {
    if(!proLicensed)
        return kAppIconResource;
    if(proStyle < 0 || proStyle >= kProIconStyleCount)
        proStyle = 0;
    return kProIconStyles[proStyle].icon;
}

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
