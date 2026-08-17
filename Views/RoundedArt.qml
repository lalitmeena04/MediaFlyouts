import QtQuick
import QtQuick.Effects

// Album art clipped to rounded corners. A Rectangle's `clip` only clips to its
// rectangular bounds, so source art with baked-in rounded corners would otherwise
// show its (often white) square corners; this masks the image to a rounded shape.
Item {
    id: root

    property url source
    property int fillMode: Image.PreserveAspectCrop
    property real radius: 8

    Image {
        id: img
        anchors.fill: parent
        source: root.source
        fillMode: root.fillMode
        visible: false
        cache: false
        asynchronous: true
    }

    Item {
        id: mask
        anchors.fill: parent
        layer.enabled: true
        visible: false
        Rectangle {
            anchors.fill: parent
            radius: root.radius
        }
    }

    MultiEffect {
        anchors.fill: parent
        source: img
        maskEnabled: true
        maskSource: mask
    }
}
