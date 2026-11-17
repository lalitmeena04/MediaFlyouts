import QtQuick

// Stand-ins for the Windows controllers. Each object-valued property becomes
// a context property of the same name. Members mirror what the views read.
QtObject {
    id: world

    // Values every part starts from.
    readonly property var defaults: ({
        "pro": {
            "mixerEnabled": true, "nextUpStyle": 0, "peripheralFlyoutEnabled": true,
            "peripheralsWidgetEnabled": true, "taskbarWidgetAdaptiveColors": false,
            "taskbarWidgetControlsEnabled": true, "taskbarWidgetLayout": 0,
            "taskbarWidgetPauseAllEnabled": true, "taskbarWidgetPlayAllEnabled": true,
            "taskbarWidgetScrollingEnabled": false, "taskbarWidgetShowPauseOverlay": true,
            "visualizerEnabled": true
        },
        "volumeController": { "volume": 62, "muted": false },
        "lockKeys": { "keyType": 0, "keyName": "Caps Lock", "on": true },
        "taskbarBackdrop": { "light": false, "luminance": 0.1 },
        "volumeMixer": { "sessions": [] },
        "audioCapture": { "active": true, "bands": [], "level": 0.5 },
        "batteryStatus": { "devices": [] },
        "peripheralAlerts": { "pending": [] },
        "powerStatus": { "bucket": "Good", "percent": 80 }
    })

    function reset() {
        for(var group in defaults)
            for(var key in defaults[group])
                world[group][key] = defaults[group][key]
    }

    Component.onCompleted: reset()

    property QtObject log: QtObject {
        function debug(message) {}
        function info(message) {}
        function warn(message) { console.warn(message) }
        function error(message) { console.error(message) }
    }

    property QtObject pro: QtObject {
        property bool mixerEnabled
        property int nextUpStyle
        property bool peripheralFlyoutEnabled
        property bool peripheralsWidgetEnabled
        property bool taskbarWidgetAdaptiveColors
        property bool taskbarWidgetControlsEnabled
        property int taskbarWidgetLayout
        property bool taskbarWidgetPauseAllEnabled
        property bool taskbarWidgetPlayAllEnabled
        property bool taskbarWidgetScrollingEnabled
        property bool taskbarWidgetShowPauseOverlay
        property bool visualizerEnabled
    }

    property QtObject capabilities: QtObject {
        readonly property bool audioAvailable: true
        readonly property bool chargerAvailable: true
        readonly property bool lockKeysAvailable: true
        readonly property bool mediaSessionAvailable: true
    }

    property QtObject monitors: QtObject {
        signal changed()
        function geometry(monitor) { return Qt.rect(0, 0, 1920, 1080) }
    }

    property QtObject fullscreen: QtObject {
        function isFullscreen(monitor) { return false }
        function isLocked() { return false }
        function shouldBlock(enabled, monitor) { return false }
    }

    property QtObject swipe: QtObject {
        function lockAxis(dx, dy, axis) { return 0 }
        function minDistance() { return 80 }
        function project(dx, dy, axis) { return Qt.point(0, 0) }
        function recognize(dx, dy, elapsedMs) { return 0 }
        function vector(direction) { return Qt.point(0, 0) }
    }

    property QtObject windowEffects: QtObject {
        function pinTopmost(window) {}
        function setDarkFrame(window, dark) {}
    }

    property QtObject taskbar: QtObject {
        function pinAboveTaskbar(window) {}
        function pinTopmost(window) {}
        function taskbarEdge(monitor) { return 0 }
        function widgetFits(monitor) { return true }
        // Far off every screen. The widgets are never shown anyway.
        function widgetPosition(monitor, side, offset, width, height) { return Qt.point(-32000, -32000) }
    }

    property QtObject taskbarBackdrop: QtObject {
        property bool light
        property real luminance
        function readable(color, light) { return color }
        function sample(window) {}
    }

    property QtObject volumeController: QtObject {
        property int volume
        property bool muted
        signal volumeChangedExternally()
        function setVolume(value) { volume = value }
        function toggleMute() { muted = !muted }
    }

    property QtObject volumeMixer: QtObject {
        property var sessions: []
        function refresh() {}
        function setSessionMute(index, muted) {}
        function setSessionVolume(index, volume) {}
    }

    property QtObject audioCapture: QtObject {
        property bool active
        property var bands: []
        property real level
        signal beat()
        function start() {}
        function stop() {}
    }

    property QtObject lockKeys: QtObject {
        property int keyType
        property string keyName
        property bool on
        signal lockKeyToggled()
    }

    property QtObject batteryStatus: QtObject {
        property var devices: []
    }

    property QtObject peripheralAlerts: QtObject {
        property var pending: []
        signal alertsRaised()
    }

    property QtObject powerStatus: QtObject {
        property string bucket
        property int percent
    }
}
