import QtQuick
import QtQuick.Controls.Basic

// Device row for the peripherals page. The name is editable, because a dongle
// often reports something unhelpful like "2.4G Wireless Receiver".
Item {
    id: row

    // Detected name. Stays the device identity even after a rename.
    required property string deviceName
    property string description: ""
    property bool value: false
    signal toggled(bool value)

    readonly property string displayName: settings.peripheralNames[row.deviceName] || row.deviceName
    readonly property bool renamed: displayName !== deviceName
    property bool editing: false

    width: parent ? parent.width : 400
    height: description.length > 0 ? 58 : 44

    function commit(text) {
        settings.setPeripheralDisplayName(row.deviceName, text)
        row.editing = false
    }

    Column {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - sw.width - renameButton.width - 32
        spacing: 3

        Item {
            width: parent.width
            height: 18

            Text {
                visible: !row.editing
                width: parent.width
                text: row.displayName
                color: theme.textPrimary
                font.pixelSize: 14
                elide: Text.ElideRight
            }

            TextField {
                id: nameField
                visible: row.editing
                width: Math.min(parent.width, 260)
                height: 24
                anchors.verticalCenter: parent.verticalCenter
                color: theme.textPrimary
                font.pixelSize: 13
                padding: 4
                background: Rectangle {
                    radius: 4
                    color: theme.card
                    border.color: theme.accent
                }
                onAccepted: row.commit(text)
                // Clicking elsewhere keeps the edit rather than dropping it.
                onActiveFocusChanged: if(!activeFocus && row.editing) row.commit(text)
                Keys.onEscapePressed: row.editing = false
            }
        }

        Text {
            visible: row.description.length > 0
            width: parent.width
            text: row.renamed ? row.description + "  \u00B7  Detected as " + row.deviceName : row.description
            color: theme.textTertiary
            font.pixelSize: 11
            elide: Text.ElideRight
        }
    }

    Rectangle {
        id: renameButton
        anchors.right: sw.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        width: 28
        height: 28
        radius: 6
        color: renameMouse.containsMouse ? theme.cardHover : "transparent"

        IconLabel {
            anchors.centerIn: parent
            glyph: Fluent.settings
            size: 13
            color: row.renamed ? theme.accent : theme.textSecondary
        }
        MouseArea {
            id: renameMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if(row.editing) {
                    row.commit(nameField.text)
                    return
                }
                nameField.text = row.displayName
                row.editing = true
                nameField.forceActiveFocus()
                nameField.selectAll()
            }
            InfoTooltip {
                text: row.renamed ? "Rename. Clear the box to restore \"" + row.deviceName + "\"." : "Rename this device"
                hovered: renameMouse.containsMouse
            }
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
