import QtQuick

// Peek gesture for flyouts. Hover holds the flyout open, leaving lets it hide.
// A quick drag in any direction swipes the flyout away. Mouse and touch.
MouseArea {
    id: peek

    // Flyout that owns the auto-hide timer.
    property var flyout: null
    // True keeps the flyout open after the pointer leaves.
    property bool pinned: false
    // Swipe can be switched off in settings. Hover and click still work.
    property bool swipeEnabled: settings.flyoutSwipeDismissEnabled

    // Press start in screen coordinates. The window moves under the pointer
    // during a drag, so window or local coordinates would feed back into the offset.
    property point pressGlobal: Qt.point(0, 0)
    property double pressTime: 0
    property bool dragging: false
    // True after a release that counted as a swipe. Blocks click and exit handling.
    property bool swiped: false

    anchors.fill: parent
    hoverEnabled: true

    function globalDelta(mouse) {
        var p = peek.mapToGlobal(mouse.x, mouse.y)
        return Qt.point(p.x - peek.pressGlobal.x, p.y - peek.pressGlobal.y)
    }

    onEntered: if(peek.flyout) peek.flyout.stopAutoHide()
    onExited: if(peek.flyout && !peek.pinned && !peek.swiped) peek.flyout.restartAutoHide()
    onClicked: if(peek.flyout && !peek.pinned && !peek.swiped) peek.flyout.restartAutoHide()

    onPressed: (mouse) => {
        peek.pressGlobal = peek.mapToGlobal(mouse.x, mouse.y)
        peek.pressTime = Date.now()
        peek.dragging = false
        peek.swiped = false
    }
    onPositionChanged: (mouse) => {
        if(!peek.pressed || !peek.swipeEnabled || !peek.flyout)
            return
        var d = peek.globalDelta(mouse)
        peek.dragging = true
        peek.flyout.dragSwipe(d.x, d.y)
    }
    onReleased: (mouse) => {
        if(!peek.flyout || !peek.dragging)
            return
        var d = peek.globalDelta(mouse)
        // The flyout owns the axis lock and the thresholds. It reports the result.
        peek.swiped = peek.flyout.releaseSwipe(d.x, d.y, Date.now() - peek.pressTime) !== 0
        peek.dragging = false
    }
    onCanceled: {
        if(peek.dragging && peek.flyout)
            peek.flyout.releaseSwipe(0, 0, 0)
        peek.dragging = false
    }
}
