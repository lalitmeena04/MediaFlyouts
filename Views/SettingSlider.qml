import QtQuick
import QtQuick.Controls.Basic

// A labelled slider row used in the dashboard settings pages.
Item {
    id: root
    property string label: ""
    property string description: ""
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property real value: 0
    property string valueText: ""
    signal moved(real value)

    width: parent ? parent.width : 400
    height: description.length > 0 ? 58 : 44

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - slider.width - valueLabel.width - 32
        spacing: 3

        Text {
            text: root.label
            color: "white"
            font.pixelSize: 14
        }
        Text {
            visible: root.description.length > 0
            width: parent.width
            text: root.description
            color: "#9AA0A6"
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
        color: "#9AA0A6"
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
