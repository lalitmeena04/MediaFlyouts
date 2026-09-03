import QtQuick
import QtQuick.Controls.Basic

// Theme-aware tooltip shown after a hover delay. Bind `hovered` to the control's
// hover state; it appears to the right of its parent once the delay elapses.
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

    // show only after the pointer rests on the control for a moment
    Timer {
        interval: 750
        running: control.hovered
        onTriggered: control.open()
    }
    onHoveredChanged: if(!hovered) close()
}
