import QtQuick

// App logo for headers and the About page. One source for every size.
Image {
    property int size: 24

    width: size
    height: size
    source: "qrc:/icons/icon-256.png"
    fillMode: Image.PreserveAspectFit
    smooth: true
    // Downscale from 256 px with mipmaps. Keeps edges clean at 24 px.
    mipmap: true
    asynchronous: false

    onStatusChanged: if(status === Image.Error) console.warn("AppLogo: failed to load", source)
}
