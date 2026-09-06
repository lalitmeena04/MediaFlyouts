import QtQuick
import QtQuick.Controls.Basic

// Theme-aware tooltip with hover delay.
// Appears to right of parent.
ToolTip {
    id: control
    property bool hovered: false

    padding: 8
    font.pixelSize: 12
    x: parent ? parent.width + 6 : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    contentItem: Text {
        text: control.text
        color: theme.textPrimary
        font: control.font
    }
    background: Rectangle {
        color: theme.flyout
        border.color: theme.flyoutBorder
        radius: 6
    }

    // Show after pointer rests on control.
    Timer {
        interval: 750
        running: control.hovered
        onTriggered: control.open()
    }
    onHoveredChanged: if(!hovered) close()
}
