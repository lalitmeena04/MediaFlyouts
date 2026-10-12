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
    Behavior on y {
        enabled: base.positioned
        NumberAnimation { duration: Anim.moveMs; easing.type: Easing.OutQuad }
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
            base.afterHide()
        }
    }
}
