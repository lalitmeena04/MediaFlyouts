import QtQuick
import QtQuick.Controls.Basic

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
        // ComboBox resets currentIndex to 0 when the model is replaced,
        // e.g. the monitor list after a display change. Re-bind the selection.
        onModelChanged: currentIndex = Qt.binding(function() { return root.currentIndex })
        onActivated: root.activated(currentIndex)
    }
}
