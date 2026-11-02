import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root
    property string label: ""
    property string description: ""
    property alias model: combo.model
    property int currentIndex: 0
    property bool locked: false
    signal activated(int index)

    readonly property string detail: root.locked ? qsTr("Pro feature. Get Pro on the Home page to use it.") : root.description

    width: parent ? parent.width : 400
    height: detail.length > 0 ? 58 : 44
    enabled: !root.locked
    opacity: root.locked ? 0.5 : 1.0

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - combo.width - 24
        spacing: 3

        Row {
            spacing: 8
            Text {
                text: root.label
                color: theme.textPrimary
                font.pixelSize: 14
            }
            ProBadge {
                visible: root.locked
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        Text {
            visible: root.detail.length > 0
            width: parent.width
            text: root.detail
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
