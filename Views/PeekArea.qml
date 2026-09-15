import QtQuick

// Peek gesture for flyouts. Hover holds the flyout open, leaving lets it hide.
MouseArea {
    id: peek

    // Flyout that owns the auto-hide timer.
    property var flyout: null
    // True keeps the flyout open after the pointer leaves.
    property bool pinned: false

    anchors.fill: parent
    hoverEnabled: true

    onEntered: if(peek.flyout) peek.flyout.stopAutoHide()
    onExited: if(peek.flyout && !peek.pinned) peek.flyout.restartAutoHide()
    onClicked: if(peek.flyout && !peek.pinned) peek.flyout.restartAutoHide()
}
