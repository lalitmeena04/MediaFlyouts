import QtQuick
import MediaFlyouts

// Bind a Translate on the target to `offset`. Then call enter(), leave() or replace().
Item {
    id: fx
    property Item target: null
    property int distance: 14
    property int direction: 1
    property int durationMs: Anim.moveMs
    property real offset: 0
    readonly property bool running: enterAnim.running || leaveAnim.running
    signal finishedLeave()

    property var pendingSwap: null

    function reset() {
        enterAnim.stop()
        leaveAnim.stop()
        fx.pendingSwap = null
        fx.offset = 0
        if(fx.target)
            fx.target.opacity = 1
    }
    function enter() {
        leaveAnim.stop()
        fx.pendingSwap = null
        fx.offset = fx.direction * fx.distance
        if(fx.target)
            fx.target.opacity = 0
        enterAnim.restart()
    }
    function leave() {
        enterAnim.stop()
        fx.pendingSwap = null
        leaveAnim.restart()
    }
    function replace(swap) {
        enterAnim.stop()
        fx.pendingSwap = swap
        leaveAnim.restart()
    }

    ParallelAnimation {
        id: enterAnim
        NumberAnimation { target: fx; property: "offset"; to: 0; duration: fx.durationMs; easing.type: Easing.OutCubic }
        NumberAnimation { target: fx.target; property: "opacity"; to: 1; duration: fx.durationMs }
    }
    ParallelAnimation {
        id: leaveAnim
        NumberAnimation {
            target: fx; property: "offset"; to: -fx.direction * fx.distance
            duration: fx.durationMs; easing.type: Easing.InCubic
        }
        NumberAnimation { target: fx.target; property: "opacity"; to: 0; duration: fx.durationMs }
        onFinished: {
            if(fx.pendingSwap) {
                var swap = fx.pendingSwap
                fx.pendingSwap = null
                swap()
                fx.enter()
            } else {
                fx.finishedLeave()
            }
        }
    }
}
