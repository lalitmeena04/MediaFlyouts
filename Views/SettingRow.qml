import QtQuick
import QtQuick.Controls.Basic

// A labelled toggle row used in the dashboard settings pages.
Item {
    id: row
    property string label: ""
    property string description: ""
    property bool value: false
    // when false the row dims, its switch is disabled, and unavailableNote replaces the description
    property bool available: true
    property string unavailableNote: ""
    signal toggled(bool value)

    readonly property bool showNote: !row.available && row.unavailableNote.length > 0
    readonly property string detail: row.showNote ? row.unavailableNote : row.description

    width: parent ? parent.width : 400
    height: detail.length > 0 ? 58 : 44
    enabled: row.available
    opacity: row.available ? 1.0 : 0.5

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - sw.width - 24
        spacing: 3

        Text {
            text: row.label
            color: theme.textPrimary
            font.pixelSize: 14
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
        enabled: row.available
        onToggled: row.toggled(checked)
    }
}
