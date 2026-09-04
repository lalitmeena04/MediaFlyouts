import QtQuick
import QtQuick.Window
import MediaFlyouts

// Shared window chrome, fade in/out and auto-hide timing for every flyout. Derived flyouts
// bind hideInterval to their own duration setting, override showFlyout() to apply their
// guards and then call present(); onExit/hover handlers use stopAutoHide()/restartAutoHide().
Window {
    id: base
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    // auto-hide delay; each flyout binds its own duration setting
    property int hideInterval: settings.flyoutDuration

    // true once App has placed this flyout, so re-stacking animates while the first show snaps
    property bool positioned: false
    Behavior on y {
        enabled: base.positioned
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    // shows the flyout with the auto-hide countdown running; derived flyouts call this once
    // their own guard checks have passed
    function present() {
        hideTimer.restart()
        base.visible = true
        fadeIn.restart()
    }

    // default entry point; derived flyouts override this with their guards + present()
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

    // hook invoked once the fade-out completes; overridden where extra teardown is needed
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
