import QtQuick
import QtQuick.Controls.Basic

// A labelled toggle row used in the dashboard settings pages.
Item {
    id: row
    property string label: ""
    property string description: ""
    property bool value: false
    signal toggled(bool value)

    width: parent ? parent.width : 400
    height: description.length > 0 ? 58 : 44

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - sw.width - 24
        spacing: 3

        Text {
            text: row.label
            color: "white"
            font.pixelSize: 14
        }
        Text {
            visible: row.description.length > 0
            width: parent.width
            text: row.description
            color: "#9AA0A6"
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }

    Switch {
        id: sw
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        checked: row.value
        onToggled: row.toggled(checked)
    }
}
