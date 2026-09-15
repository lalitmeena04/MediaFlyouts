import QtQuick
import QtQuick.Window
import MediaFlyouts

// Small always-on-top battery widget for selected peripherals.
// Docks on taskbar like TaskbarWidget.qml.
DockableWidget {
    id: root
    logName: "PeripheralsWidget"
    pollActive: settings.peripheralsWidgetEnabled
    // Now-playing taskbar widget from App.qml.
    // Used to dock next to it without overlap.
    property var mediaWidget: null

    readonly property var devices: {
        var picked = []
        var all = batteryStatus.devices
        // Read property directly. Binding updates when Dashboard selection changes.
        var excluded = settings.peripheralsWidgetExcluded
        var wiredOn = settings.peripheralsWidgetWiredEnabled
        for(var i = 0; i < all.length; i++) {
            if(all[i].connectionKind === "usb" && !wiredOn)
                continue
            if(excluded.indexOf(all[i].name) === -1)
                picked.push(all[i])
        }
        return picked
    }
    readonly property var dongleDevices: devices.filter(function(device) {
        return device.connectionKind === "wireless"
    })
    readonly property var bluetoothDevices: devices.filter(function(device) {
        return device.connectionKind === "bluetooth" || device.connectionKind === "ble"
    })
    readonly property var wiredDevices: devices.filter(function(device) {
        return device.connectionKind === "usb"
    })
    readonly property var deviceSections: [
        { "label": "2.4 GHz", "devices": dongleDevices },
        { "label": "Bluetooth", "devices": bluetoothDevices },
        { "label": "Wired", "devices": wiredDevices }
    ].filter(function(section) { return section.devices.length > 0 })

    function sectionGlyph(label) {
        if(label === "Bluetooth")
            return Fluent.bluetooth
        if(label === "Wired")
            return Fluent.plugConnected
        return Fluent.usbStick
    }

    // One ring per device, one icon per section, one separator between sections.
    width: {
        var total = 0
        for(var i = 0; i < deviceSections.length; i++)
            total += deviceSections[i].devices.length * 32 + 22
        return Math.max(32, total + Math.max(0, deviceSections.length - 1) * 14)
    }
    height: 32

    // Show after onboarding, when enabled, devices exist, and not blocked.
    visible: settings.onboardingCompleted && settings.peripheralsWidgetEnabled && devices.length > 0 && !root.blocked

    // True after first placement. Later moves animate.
    property bool positioned: false
    Behavior on x {
        enabled: root.positioned
        NumberAnimation { duration: Anim.moveMs; easing.type: Easing.OutQuad }
    }

    function updatePosition() {
        // Share media widget monitor and taskbar edge.
        // Reserve space only while media widget is visible.
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

    onVisibleChanged: {
        log.debug("PeripheralsWidget.visible=" + visible
            + " onboardingCompleted=" + settings.onboardingCompleted
            + " enabled=" + settings.peripheralsWidgetEnabled
            + " deviceCount=" + devices.length
            + " blocked=" + root.blocked
            + " monitor=" + settings.taskbarWidgetMonitor)
        if(visible)
            updatePosition()
    }
    onWidthChanged: updatePosition()

    Connections {
        target: batteryStatus
        function onDevicesChanged() { root.updatePosition() }
    }
    Connections {
        target: root.mediaWidget
        function onVisibleChanged() { root.updatePosition() }
        function onWidthChanged() { root.updatePosition() }
    }

    // Popup or ToolTip would clip to this window bounds.
    // Use small top-level window for hover tooltip.
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
                            text: root.sectionGlyph(modelData.label)
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
                                    showGauge: DeviceIcons.hasBattery(modelData.connectionKind)

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
                                                    var tipText = settings.peripheralNames[modelData.name] || modelData.name
                                                    if(DeviceIcons.hasBattery(modelData.connectionKind)) {
                                                        tipText += modelData.percent >= 0 ? " \u2014 " + modelData.percent + "%" : " \u2014 battery unknown"
                                                        if(modelData.charging)
                                                            tipText += " \u26A1"
                                                    }
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
