import QtQuick

MouseArea {
    id: peek

    property var flyout: null
    property bool pinned: false
    property bool swipeEnabled: settings.flyoutSwipeDismissEnabled

    // Screen coordinates. The window moves under the pointer during a drag.
    property point pressGlobal: Qt.point(0, 0)
    property double pressTime: 0
    property bool dragging: false
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
        peek.swiped = peek.flyout.releaseSwipe(d.x, d.y, Date.now() - peek.pressTime) !== 0
        peek.dragging = false
    }
    onCanceled: {
        if(peek.dragging && peek.flyout)
            peek.flyout.releaseSwipe(0, 0, 0)
        peek.dragging = false
    }
}
