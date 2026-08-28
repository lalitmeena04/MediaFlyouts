import QtQuick
import QtQuick.Window
import MediaFlyouts

// A small always-on-top battery widget for user-selected peripherals, docked on the
// taskbar like TaskbarWidget.qml.
Window {
    id: root
    // the now-playing taskbar widget, set by App.qml, so this widget can dock next to
    // it instead of overlapping when both share the same monitor/side
    property var mediaWidget: null

    readonly property var devices: {
        var picked = []
        var all = batteryStatus.devices
        // read the property directly (not the peripheralSelected() method) so this
        // binding actually re-evaluates when the selection changes in the Dashboard
        var selected = settings.peripheralsWidgetSelected
        for(var i = 0; i < all.length; i++)
            if(selected.indexOf(all[i].name) !== -1)
                picked.push(all[i])
        return picked
    }

    width: Math.max(1, devices.length) * 40
    height: 32
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus

    // shown only once onboarding is done, when enabled, at least one selected device is currently
    // reporting battery, and not blocked by fullscreen / lock screen
    visible: settings.onboardingCompleted && settings.peripheralsWidgetEnabled && devices.length > 0 && !root.blocked

    // fullscreen/lock state is polled (shouldBlock has no change signal) so this can't be a binding
    property bool blocked: false

    function updatePosition() {
        // if the media widget is showing on the same monitor/edge, dock right after it
        // instead of on top of it
        var extraOffset = 0
        if(root.mediaWidget && root.mediaWidget.visible
           && settings.peripheralsWidgetMonitor === settings.taskbarWidgetMonitor
           && settings.peripheralsWidgetSide === settings.taskbarWidgetSide) {
            extraOffset = root.mediaWidget.width + 8
        }
        var p = taskbar.widgetPosition(settings.peripheralsWidgetMonitor, settings.peripheralsWidgetSide,
                                       settings.peripheralsWidgetOffset + extraOffset, width, height)
        root.x = p.x
        root.y = p.y
        if(root.visible)
            taskbar.pinAboveTaskbar(root)
    }

    Component.onCompleted: updatePosition()
    onVisibleChanged: if(visible) updatePosition()
    onWidthChanged: updatePosition()

    Connections {
        target: settings
        function onChanged() { root.updatePosition() }
    }
    Connections {
        target: batteryStatus
        function onDevicesChanged() { root.updatePosition() }
    }
    Connections {
        target: root.mediaWidget
        function onVisibleChanged() { root.updatePosition() }
        function onWidthChanged() { root.updatePosition() }
    }

    Timer {
        interval: 500
        running: settings.peripheralsWidgetEnabled
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            root.blocked = fullscreen.shouldBlock(settings.disableOnFullscreen)
            root.updatePosition()
        }
    }

    // a plain Popup/ToolTip would be clipped to this tiny frameless window's own bounds,
    // so the hover tooltip is its own small top-level window instead
    Window {
        id: tip
        flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus
        color: "transparent"
        width: tipText.implicitWidth + 16
        height: tipText.implicitHeight + 10

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: "#333333"
            border.color: "#4A4A4A"

            Text {
                id: tipText
                anchors.centerIn: parent
                color: "#EEEEEE"
                font.pixelSize: 11
            }
        }
    }

    function showTip(globalPoint, text) {
        tipText.text = text
        tip.x = globalPoint.x - tip.width / 2
        tip.y = globalPoint.y - tip.height - 6
        tip.visible = true
    }
    function hideTip() {
        tip.visible = false
    }

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: "#2A2A2A"

        Row {
            anchors.centerIn: parent
            spacing: 6

            Repeater {
                model: root.devices
                delegate: BatteryRing {
                    id: deviceRing
                    required property var modelData
                    width: 26
                    height: 26
                    thickness: 3
                    percent: modelData.percent

                    Text {
                        anchors.centerIn: parent
                        text: DeviceIcons.glyph(modelData.name, modelData.connectionKind, modelData.category)
                        font.pixelSize: 13
                    }

                    // HoverHandler isn't a QQuickItem, so it needs its own Item wrapper
                    // to be accepted by BatteryRing's item-only 'content' default property
                    Item {
                        anchors.fill: parent

                        HoverHandler {
                            onHoveredChanged: {
                                if(hovered)
                                    root.showTip(deviceRing.mapToGlobal(deviceRing.width / 2, 0),
                                                 modelData.name + " \u2014 " + modelData.percent + "%")
                                else
                                    root.hideTip()
                            }
                        }
                    }
                }
            }
        }
    }
}
