import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: base
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus

    property bool blocked: false
    property int dockMonitor: settings.taskbarWidgetMonitor
    property bool pollActive: false
    property string logName: "DockableWidget"

    onWidthChanged: base.updatePosition()
    onHeightChanged: base.updatePosition()

    function updatePosition() {
        var p = taskbar.widgetPosition(base.dockMonitor, settings.taskbarWidgetSide,
                                       settings.taskbarWidgetOffset, width, height)
        base.x = p.x
        base.y = p.y
        if(base.visible)
            taskbar.pinAboveTaskbar(base)
    }

    Component.onCompleted: base.updatePosition()

    Connections {
        target: settings
        function onChanged() { base.updatePosition() }
    }

    Connections {
        target: monitors
        function onChanged() {
            log.debug(base.logName + ": monitor layout changed, size=" + base.width + "x" + base.height
                + " dpr=" + Screen.devicePixelRatio)
            base.updatePosition()
        }
    }

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
