import QtQuick
import QtQuick.Controls.Basic

Item {
    id: row
    property string label: ""
    property string description: ""
    property bool value: false
    property bool available: true
    property string unavailableNote: ""
    property bool locked: false
    signal toggled(bool value)

    readonly property bool showNote: !row.available && row.unavailableNote.length > 0
    readonly property string detail: row.locked ? qsTr("Pro feature. Get Pro on the Home page to use it.")
                                                : (row.showNote ? row.unavailableNote : row.description)
    readonly property bool usable: row.available && !row.locked

    width: parent ? parent.width : 400
    height: detail.length > 0 ? 58 : 44
    enabled: row.usable
    opacity: row.usable ? 1.0 : 0.5

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - sw.width - 24
        spacing: 3

        Row {
            spacing: 8
            Text {
                text: row.label
                color: theme.textPrimary
                font.pixelSize: 14
            }
            ProBadge {
                visible: row.locked
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        Text {
            visible: row.detail.length > 0
            width: parent.width
            text: row.detail
            color: theme.textTertiary
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }

    Switch {
        id: sw
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        checked: row.value
        enabled: row.usable
        onToggled: row.toggled(checked)
    }
}
