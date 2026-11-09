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

    // Widget colors. They follow the taskbar backdrop when adaptive colors are on,
    // so text stays readable over a transparent taskbar or a live wallpaper.
    readonly property bool adaptiveColors: pro.taskbarWidgetAdaptiveColors
    readonly property bool lightBackdrop: base.adaptiveColors && taskbarBackdrop.light
    readonly property color fgPrimary: base.lightBackdrop ? "#1A1A1A" : "#FFFFFF"
    readonly property color fgSecondary: base.lightBackdrop ? "#555555" : "#AAAAAA"
    readonly property color fgControl: base.lightBackdrop ? "#3C3C3C" : "#C8C8C8"
    readonly property color fgMuted: base.lightBackdrop ? "#8A8A8A" : "#5A5A5A"
    readonly property color hoverBg: base.lightBackdrop ? "#1F000000" : "#2A2A2A"
    readonly property color artPlaceholder: base.lightBackdrop ? "#D0D0D0" : "#333333"
    readonly property color divider: base.lightBackdrop ? "#33000000" : theme.border
    readonly property color accent: base.adaptiveColors
        ? taskbarBackdrop.readable(theme.accent, taskbarBackdrop.light)
        : theme.accent

    onLightBackdropChanged: log.debug(base.logName + ".lightBackdrop=" + base.lightBackdrop
        + " luminance=" + taskbarBackdrop.luminance.toFixed(2))

    Timer {
        interval: 500
        running: base.visible && base.adaptiveColors
        repeat: true
        triggeredOnStart: true
        onTriggered: taskbarBackdrop.sample(base)
    }

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
            // A horizontal strip cannot sit on a left or right taskbar.
            base.blocked = fullscreen.isLocked() || fullscreen.isFullscreen(base.dockMonitor)
                || !taskbar.widgetFits(base.dockMonitor)
            if(wasBlocked !== base.blocked)
                log.debug(base.logName + ".blocked changed " + wasBlocked + " -> " + base.blocked
                    + " (monitor=" + base.dockMonitor + " taskbarEdge=" + taskbar.taskbarEdge(base.dockMonitor) + ")")
            base.updatePosition()
        }
    }
}
