import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root
    property string label: ""
    property string description: ""
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property real value: 0
    property string valueText: ""
    property bool parentLocked: false
    signal moved(real value)

    width: parent ? parent.width : 400
    height: description.length > 0 ? 58 : 44
    enabled: !root.parentLocked
    opacity: root.parentLocked ? 0.5 : 1.0

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - slider.width - valueLabel.width - 32
        spacing: 3

        Text {
            text: root.label
            color: theme.textPrimary
            font.pixelSize: 14
        }
        Text {
            visible: root.description.length > 0
            width: parent.width
            text: root.description
            color: theme.textTertiary
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }

    Text {
        id: valueLabel
        anchors.right: slider.left
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: root.valueText
        color: theme.textTertiary
        font.pixelSize: 12
    }

    Slider {
        id: slider
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 180
        from: root.from
        to: root.to
        stepSize: root.stepSize
        value: root.value
        onMoved: root.moved(value)
    }
}
