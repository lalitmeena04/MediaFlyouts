import QtQuick
import QtQuick.Window
import MediaFlyouts

// Shared chrome, taskbar docking and fullscreen/lock blocking for the always-on-top taskbar
// widgets. Derived widgets bind pollActive (when to keep polling) and logName, and may override
// updatePosition() for custom docking next to a sibling widget.
Window {
    id: base
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus

    // fullscreen/lock state is polled (no change signal) so this can't be a binding
    property bool blocked: false
    // the monitor this widget docks on; drives both placement and the block check
    property int dockMonitor: settings.taskbarWidgetMonitor
    // keep the 500ms poll running while the widget is enabled (even when hidden), so it
    // reappears once a fullscreen app or the lock screen goes away
    property bool pollActive: false
    // identifies this widget in the debug log
    property string logName: "DockableWidget"

    // docks at the configured taskbar edge and keeps the widget above the taskbar; overridden by
    // widgets that need custom placement (e.g. docking next to another widget)
    function updatePosition() {
        var p = taskbar.widgetPosition(base.dockMonitor, settings.taskbarWidgetSide,
                                       settings.taskbarWidgetOffset, width, height)
        base.x = p.x
        base.y = p.y
        // keep the widget above the (possibly transparent) taskbar so it stays clickable
        if(base.visible)
            taskbar.pinAboveTaskbar(base)
    }

    Component.onCompleted: base.updatePosition()

    // reposition immediately when the offset (or other settings) change
    Connections {
        target: settings
        function onChanged() { base.updatePosition() }
    }

    // poll fullscreen/lock state and re-assert placement/z-order while active, even when hidden,
    // so the widget reappears once the fullscreen app or lock screen is gone
    Timer {
        interval: 500
        running: base.pollActive
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            var wasBlocked = base.blocked
            base.blocked = fullscreen.isLocked() || fullscreen.isFullscreen(base.dockMonitor)
            if(wasBlocked !== base.blocked)
                log.debug(base.logName + ".blocked changed " + wasBlocked + " -> " + base.blocked
                    + " (monitor=" + base.dockMonitor + ")")
            base.updatePosition()
        }
    }
}
