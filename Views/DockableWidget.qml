import QtQuick
import QtQuick.Window
import MediaFlyouts

// Shared chrome, taskbar docking, and fullscreen blocking for taskbar widgets.
// Derived widgets bind pollActive and logName. They can override updatePosition().
Window {
    id: base
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus

    // Fullscreen/lock state is polled. No change signal exists.
    property bool blocked: false
    // Monitor for widget docking and block checks.
    property int dockMonitor: settings.taskbarWidgetMonitor
    // Keep 500 ms poll active while widget is enabled.
    // Widget can reappear after fullscreen or lock screen.
    property bool pollActive: false
    // Widget name in debug log.
    property string logName: "DockableWidget"

    // Dock at configured taskbar edge. Keep widget above taskbar.
    // Widgets can override for custom placement.
    function updatePosition() {
        var p = taskbar.widgetPosition(base.dockMonitor, settings.taskbarWidgetSide,
                                       settings.taskbarWidgetOffset, width, height)
        base.x = p.x
        base.y = p.y
        // Keep widget above taskbar. It stays clickable.
        if(base.visible)
            taskbar.pinAboveTaskbar(base)
    }

    Component.onCompleted: base.updatePosition()

    // Reposition when settings change.
    Connections {
        target: settings
        function onChanged() { base.updatePosition() }
    }

    // Poll fullscreen/lock state. Reassert placement and z-order while active.
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
