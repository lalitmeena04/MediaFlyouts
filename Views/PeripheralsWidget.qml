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
        var excluded = settings.peripheralsWidgetExcluded
        for(var i = 0; i < all.length; i++)
            if(excluded.indexOf(all[i].name) === -1)
                picked.push(all[i])
        return picked
    }
    readonly property var dongleDevices: devices.filter(function(device) {
        return device.connectionKind === "wireless"
    })
    readonly property var bluetoothDevices: devices.filter(function(device) {
        return device.connectionKind === "bluetooth" || device.connectionKind === "ble"
    })
    readonly property var deviceSections: [
        { "label": "2.4 GHz", "devices": dongleDevices },
        { "label": "Bluetooth", "devices": bluetoothDevices }
    ].filter(function(section) { return section.devices.length > 0 })

        width: Math.max(1, dongleDevices.length) * 32 + Math.max(1, bluetoothDevices.length) * 32
                + deviceSections.length * 22 + (dongleDevices.length > 0 && bluetoothDevices.length > 0 ? 14 : 0)
            height: 32
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus

    // shown only once onboarding is done, when enabled, at least one selected device is currently
    // reporting battery, and not blocked by fullscreen / lock screen
    visible: settings.onboardingCompleted && settings.peripheralsWidgetEnabled && devices.length > 0 && !root.blocked

    // fullscreen/lock state is polled (shouldBlock has no change signal) so this can't be a binding
    property bool blocked: false

    // true once this widget has been placed once, so later moves (media widget showing up or
    // going away) slide smoothly instead of snapping
    property bool positioned: false
    Behavior on x {
        enabled: root.positioned
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }

    function updatePosition() {
        // shares the media widget's monitor and taskbar edge; only reserve space next to it
        // while it is actually showing (media playing), so this widget slides in to fill the
        // gap when nothing is playing and slides back out once playback starts again
        var extraOffset = 0
        if(root.mediaWidget && root.mediaWidget.visible)
            extraOffset = root.mediaWidget.width + 8
        var p = taskbar.widgetPosition(settings.taskbarWidgetMonitor, settings.taskbarWidgetSide,
                                       settings.taskbarWidgetOffset + extraOffset, width, height)
        root.x = p.x
        root.y = p.y
        if(root.visible)
            taskbar.pinAboveTaskbar(root)
        root.positioned = true
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
        var mon = taskbar.monitorGeometry(settings.taskbarWidgetMonitor)
        var x = globalPoint.x - tip.width / 2
        if(mon.width > 0)
            x = Math.max(mon.x + 4, Math.min(x, mon.x + mon.width - tip.width - 4))
        tip.x = x
        tip.y = globalPoint.y - tip.height - 6
        tip.visible = true
    }
    function hideTip() {
        tip.visible = false
    }

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: cardHover.hovered ? "#2A2A2A" : "transparent"

        HoverHandler {
            id: cardHover
        }

        Row {
            anchors.centerIn: parent
            spacing: 7

            Repeater {
                model: root.deviceSections
                delegate: Row {
                    required property int index
                    required property var modelData
                    spacing: 5

                    Rectangle {
                        visible: index > 0
                        width: 1
                        height: 28
                        anchors.verticalCenter: parent.verticalCenter
                        color: theme.border
                    }

                    Row {
                        spacing: 6

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.label === "Bluetooth" ? Fluent.bluetooth : Fluent.usbStick
                            font.family: Fluent.fontFamily
                            font.pixelSize: 17
                            color: theme.accent
                        }

                        Repeater {
                            model: modelData.devices
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
                                        font.family: DeviceIcons.family(modelData.name, modelData.connectionKind, modelData.category)
                                        font.pixelSize: 13
                                        color: theme.accent
                                    }

                                    Item {
                                        anchors.fill: parent

                                        HoverHandler {
                                            onHoveredChanged: {
                                                if(hovered) {
                                                    var tipText = modelData.name
                                                    tipText += modelData.percent >= 0 ? " \u2014 " + modelData.percent + "%" : " \u2014 battery unknown"
                                                    if(modelData.charging)
                                                        tipText += " \u26A1"
                                                    var link = DeviceIcons.transport(modelData.connectionKind)
                                                    if(link)
                                                        tipText += "   \u00B7 " + link
                                                    var parts = []
                                                    for(var i = 0; i < modelData.components.length; i++) {
                                                        var c = modelData.components[i]
                                                        parts.push(c.label + " " + c.percent + "%" + (c.charging ? " \u26A1" : ""))
                                                    }
                                                    if(parts.length > 1)
                                                        tipText += "\n" + parts.join("   ")
                                                    root.showTip(deviceRing.mapToGlobal(deviceRing.width / 2, 0), tipText)
                                                } else {
                                                    root.hideTip()
                                                }
                                            }
                                        }
                                    }
                                }
                        }
                    }
                }
            }
        }
    }
}
