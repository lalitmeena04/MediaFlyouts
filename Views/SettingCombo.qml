import QtQuick
import QtQuick.Controls.Basic

// A labelled dropdown row used in the dashboard settings pages.
Item {
    id: root
    property string label: ""
    property string description: ""
    property alias model: combo.model
    property int currentIndex: 0
    signal activated(int index)

    width: parent ? parent.width : 400
    height: description.length > 0 ? 58 : 44

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - combo.width - 24
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

    ComboBox {
        id: combo
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 200
        currentIndex: root.currentIndex
        onActivated: root.activated(currentIndex)
    }
}
