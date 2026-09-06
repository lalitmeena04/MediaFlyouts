import QtQuick

// Small vertical battery icon drawn with shapes.
// Level sets fill height. Color sets fill color.
Item {
    id: bat
    property color color: "white"
    property real level: 1.0
    implicitWidth: 9
    implicitHeight: 14

    readonly property real _nub: Math.max(1, Math.round(implicitHeight * 0.09))
    readonly property real _clamped: Math.max(0, Math.min(1, level))

    // Terminal nub on top.
    Rectangle {
        width: parent.width * 0.44
        height: bat._nub
        radius: height / 2
        color: bat.color
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // Body outline with bottom-anchored fill.
    Rectangle {
        id: body
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: parent.height - bat._nub
        radius: 2
        color: "transparent"
        border.color: bat.color
        border.width: 1

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 1.5
            radius: 1
            color: bat.color
            height: (body.height - 3) * bat._clamped
        }
    }
}
