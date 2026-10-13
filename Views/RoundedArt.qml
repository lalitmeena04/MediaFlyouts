import QtQuick
import QtQuick.Effects

// Mask image. Rectangle clip uses rectangular bounds.
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
