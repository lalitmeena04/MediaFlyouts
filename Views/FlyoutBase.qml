import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: base
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    property int hideInterval: settings.flyoutDuration

    property bool positioned: false

    // Stacking math uses homeX/homeY, never x/y. The window sits here plus the swipe offset.
    property real homeX: 0
    property real homeY: 0
    x: base.homeX + base.swipeDx
    y: base.homeY + base.swipeDy
    Behavior on y {
        enabled: base.positioned && !base.swiping
        NumberAnimation { duration: Anim.moveMs; easing.type: Easing.OutQuad }
    }

    default property alias content: slot.data
    property bool swiping: false
    // 0 none, 1 horizontal, 2 vertical.
    property int swipeAxis: 0
    property real swipeDx: 0
    property real swipeDy: 0
    property real swipeScale: 1
    readonly property real swipeExitScale: 0.9
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

    function placeAt(px, py) {
        base.homeX = px
        base.homeY = py
    }

    function present() {
        hideTimer.restart()
        base.visible = true
        log.debug("FlyoutBase.present size=" + base.width + "x" + base.height + " at=" + base.x + "," + base.y
            + " dpr=" + Screen.devicePixelRatio)
        windowEffects.pinTopmost(base)
        taskbar.pinTopmost(base)
        fadeIn.restart()
    }

    function showFlyout() {
        present()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

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

    function releaseSwipe(dx, dy, elapsedMs) {
        var p = swipe.project(dx, dy, base.swipeAxis)
        var direction = swipe.recognize(p.x, p.y, elapsedMs)
        if(direction !== 0)
            dismissBySwipe(direction)
        else if(base.swiping)
            springBack.restart()
        return direction
    }

    function dismissBySwipe(direction) {
        var v = swipe.vector(direction)
        log.debug("FlyoutBase.dismissBySwipe direction=" + direction)
        hideTimer.stop()
        springBack.stop()
        base.swiping = true
        flingX.to = base.swipeDx + v.x * base.width * 0.6
        flingY.to = base.swipeDy + v.y * base.height * 0.6
        slideOut.restart()
        fadeOut.restart()
    }

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

    function afterHide() {}

    Timer {
        id: hideTimer
        interval: base.hideInterval
        onTriggered: base.hideFlyout()
    }

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
            base.resetSwipe()
            base.afterHide()
        }
    }
}
