import QtQuick
import QtQuick.Window
import MediaFlyouts

// Shared window and timing for all flyouts.
// Each flyout checks its own rules before it calls present().
Window {
    id: base
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    // Auto-hide delay.
    property int hideInterval: settings.flyoutDuration

    // True after App places this flyout once.
    property bool positioned: false

    // Where App.restack() put this flyout. Stacking math uses this, never x/y.
    // The window sits here plus the swipe offset.
    property real homeX: 0
    property real homeY: 0
    x: base.homeX + base.swipeDx
    y: base.homeY + base.swipeDy
    Behavior on y {
        // The swipe moves the window itself. Keep the stacking slide out of its way.
        enabled: base.positioned && !base.swiping
        NumberAnimation { duration: Anim.moveMs; easing.type: Easing.OutQuad }
    }

    // Derived flyouts put their card here. The slot shrinks a little as the swipe goes.
    default property alias content: slot.data
    // True from the first drag move until the swipe ends or springs back.
    property bool swiping: false
    // Drag axis. 0 none, 1 horizontal, 2 vertical. Locked on the first few px of travel.
    property int swipeAxis: 0
    // Window offset from home along the locked axis, in logical px.
    property real swipeDx: 0
    property real swipeDy: 0
    // Card scale. 1 at rest, smaller as the drag passes the threshold and on the way out.
    property real swipeScale: 1
    // Scale the card reaches when the swipe completes.
    readonly property real swipeExitScale: 0.9
    // Scale the card reaches while the drag is held past the threshold.
    readonly property real swipeArmedScale: 0.96

    Item {
        id: slot
        anchors.fill: parent
        transform: Scale {
            origin.x: slot.width / 2
            origin.y: slot.height / 2
            xScale: base.swipeScale
            yScale: base.swipeScale
        }
    }

    // App.restack() calls this. Snaps when hidden, slides when shown.
    function placeAt(px, py) {
        base.homeX = px
        base.homeY = py
    }

    // Show the flyout and start auto-hide.
    function present() {
        hideTimer.restart()
        base.visible = true
        log.debug("FlyoutBase.present size=" + base.width + "x" + base.height + " at=" + base.x + "," + base.y
            + " dpr=" + Screen.devicePixelRatio)
        windowEffects.pinTopmost(base)
        taskbar.pinTopmost(base)
        fadeIn.restart()
    }

    // Default entry point.
    function showFlyout() {
        present()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

    // The window follows the pointer in a straight line along the locked axis.
    // The card shrinks and dims as the drag passes the swipe threshold, so the
    // user sees that release will dismiss.
    function dragSwipe(dx, dy) {
        if(!base.swiping) {
            base.swiping = true
            fadeIn.stop()
            springBack.stop()
            slideOut.stop()
        }
        base.swipeAxis = swipe.lockAxis(dx, dy, base.swipeAxis)
        var p = swipe.project(dx, dy, base.swipeAxis)
        base.swipeDx = p.x
        base.swipeDy = p.y
        var progress = Math.min(1, (Math.abs(p.x) + Math.abs(p.y)) / swipe.minDistance())
        base.swipeScale = 1 - (1 - base.swipeArmedScale) * progress
        base.opacity = 1 - 0.3 * progress
    }

    // Pointer released. Returns the direction, 0 when the drag was not a
    // swipe. Travel off the locked axis does not count.
    function releaseSwipe(dx, dy, elapsedMs) {
        var p = swipe.project(dx, dy, base.swipeAxis)
        var direction = swipe.recognize(p.x, p.y, elapsedMs)
        if(direction !== 0)
            dismissBySwipe(direction)
        else if(base.swiping)
            springBack.restart()
        return direction
    }

    // Fling the window along the swipe while the card shrinks and fades.
    // Works on pinned flyouts too. The flyout stays hidden until its next event.
    function dismissBySwipe(direction) {
        var v = swipe.vector(direction)
        log.debug("FlyoutBase.dismissBySwipe direction=" + direction)
        hideTimer.stop()
        springBack.stop()
        base.swiping = true
        // Travel most of the window's own size. The fade finishes first, so
        // nothing is seen crossing a screen edge.
        flingX.to = base.swipeDx + v.x * base.width * 0.6
        flingY.to = base.swipeDy + v.y * base.height * 0.6
        slideOut.restart()
        fadeOut.restart()
    }

    // Back to home. Clears every swipe effect.
    function resetSwipe() {
        slideOut.stop()
        springBack.stop()
        base.swipeDx = 0
        base.swipeDy = 0
        base.swipeAxis = 0
        base.swipeScale = 1
        base.swiping = false
    }

    function stopAutoHide() {
        hideTimer.stop()
    }

    function restartAutoHide() {
        hideTimer.restart()
    }

    // Runs after fade-out finishes.
    function afterHide() {}

    Timer {
        id: hideTimer
        interval: base.hideInterval
        onTriggered: base.hideFlyout()
    }

    // Short or slow drag. Slide back home and restore the card.
    ParallelAnimation {
        id: springBack
        NumberAnimation { target: base; property: "swipeDx"; to: 0; duration: Anim.moveMs; easing.type: Easing.OutBack }
        NumberAnimation { target: base; property: "swipeDy"; to: 0; duration: Anim.moveMs; easing.type: Easing.OutBack }
        NumberAnimation { target: base; property: "swipeScale"; to: 1; duration: Anim.moveMs; easing.type: Easing.OutQuad }
        NumberAnimation { target: base; property: "opacity"; to: 1; duration: Anim.moveMs; easing.type: Easing.OutQuad }
        onFinished: {
            base.swipeAxis = 0
            base.swiping = false
        }
    }

    ParallelAnimation {
        id: slideOut
        NumberAnimation {
            id: flingX
            target: base
            property: "swipeDx"
            duration: settings.flyoutAnimationMs
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            id: flingY
            target: base
            property: "swipeDy"
            duration: settings.flyoutAnimationMs
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: base
            property: "swipeScale"
            to: base.swipeExitScale
            duration: settings.flyoutAnimationMs
            easing.type: Easing.OutCubic
        }
    }

    NumberAnimation {
        id: fadeIn
        target: base
        property: "opacity"
        to: 1
        duration: settings.flyoutAnimationMs
        easing.type: Anim.easingFor(settings.flyoutAnimationEasingStyle)
    }

    NumberAnimation {
        id: fadeOut
        target: base
        property: "opacity"
        to: 0
        duration: settings.flyoutAnimationMs
        easing.type: Anim.easingFor(settings.flyoutAnimationEasingStyle)
        onFinished: {
            base.visible = false
            // Next present() starts from the resting place.
            base.resetSwipe()
            base.afterHide()
        }
    }
}
