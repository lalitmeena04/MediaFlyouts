import QtQuick
import QtQuick.Effects

// A grabbed view, enlarged. zoom is design units per logical pixel of the view.
Item {
    id: part

    property string name: ""
    property real zoom: 3
    property bool shadow: true

    readonly property real logicalWidth: image.sourceSize.width / showcase.partScale
    readonly property real logicalHeight: image.sourceSize.height / showcase.partScale

    width: part.logicalWidth * part.zoom
    height: part.logicalHeight * part.zoom

    Image {
        id: image
        anchors.fill: parent
        source: part.name.length > 0 ? showcase.partUrl(part.name) : ""
        asynchronous: false
        smooth: true
        mipmap: true

        layer.enabled: part.shadow
        layer.smooth: true
        layer.effect: MultiEffect {
            autoPaddingEnabled: true
            shadowEnabled: true
            shadowColor: "#000000"
            shadowOpacity: 0.55
            shadowBlur: 1.0
            blurMax: 64
            shadowVerticalOffset: 22
        }
    }
}
