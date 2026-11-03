import QtQuick

// App logo for headers and the About page. One source for every size.
// A licensed Pro user gets the Pro mark of the style they picked.
Image {
    property int size: 24
    // Same order as mf::kProIconStyles in Controllers/AppIcon.h.
    readonly property var proLogos: ["qrc:/icons/icon-pro-bar-256.png", "qrc:/icons/icon-pro-ribbon-256.png"]

    width: size
    height: size
    source: entitlement.licensed ? (proLogos[settings.proIconStyle] || proLogos[0]) : "qrc:/icons/icon-256.png"
    fillMode: Image.PreserveAspectFit
    smooth: true
    // Downscale from 256 px with mipmaps. Keeps edges clean at 24 px.
    mipmap: true
    asynchronous: false

    onStatusChanged: if(status === Image.Error) console.warn("AppLogo: failed to load", source)
}
