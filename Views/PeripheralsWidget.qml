import QtQuick
import QtQuick.Window
import MediaFlyouts

DockableWidget {
    id: root
    logName: "PeripheralsWidget"
    pollActive: settings.peripheralsWidgetEnabled
    property var mediaWidget: null
    property var stack: null
    readonly property bool stacked: root.stack !== null && root.stack.enabled

    readonly property var devices: {
        var picked = []
        var all = batteryStatus.devices
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
    readonly property string activeSection: root.stacked ? root.stack.activeSection : ""
    property string displayedSection: ""
    readonly property var shownSections: {
        if(root.displayedSection === "")
            return deviceSections
        return deviceSections.filter(function(section) { return section.label === root.displayedSection })
    }

    onStackedChanged: {
        if(!root.stacked) {
            root.displayedSection = ""
            pageFx.reset()
        }
    }
    onActiveSectionChanged: {
        if(root.activeSection === "")
            return
        if(!root.pageShown || root.displayedSection === "") {
            root.displayedSection = root.activeSection
            return
        }
        if(root.displayedSection !== root.activeSection)
            pageFx.replace(function() { root.displayedSection = root.activeSection })
    }

    function sectionGlyph(label) {
        if(label === "Bluetooth")
            return Fluent.bluetooth
        if(label === "Wired")
            return Fluent.plugConnected
        return Fluent.usbStick
    }

    function sectionWidth(section) {
        return section.devices.length * 32 + 22
    }
    readonly property int naturalWidth: {
        var total = 0
        for(var i = 0; i < shownSections.length; i++)
            total += sectionWidth(shownSections[i])
        return Math.max(32, total + Math.max(0, shownSections.length - 1) * 14)
    }
    readonly property int naturalHeight: 32
    readonly property int widestSectionWidth: {
        var widest = 32
        for(var i = 0; i < deviceSections.length; i++)
            widest = Math.max(widest, sectionWidth(deviceSections[i]))
        return widest
    }
    width: root.stacked ? root.stack.frameWidth : root.naturalWidth
    height: root.stacked ? root.stack.frameHeight : root.naturalHeight

    readonly property bool eligible: settings.onboardingCompleted && settings.peripheralsWidgetEnabled && devices.length > 0 && !root.blocked
    readonly property bool pageActive: !root.stacked || root.activeSection !== ""
    property bool pageShown: root.pageActive
    visible: root.eligible && root.pageShown

    onPageActiveChanged: {
        if(root.pageActive) {
            root.pageShown = true
            root.displayedSection = root.activeSection
            if(root.stacked)
                pageFx.enter()
            else
                pageFx.reset()
        } else {
            pageFx.leave()
        }
    }

    SlideFade {
        id: pageFx
        target: content
        direction: root.stack ? root.stack.lastStep : 1
        onFinishedLeave: root.pageShown = false
    }

    property bool positioned: false
    Behavior on x {
        enabled: root.positioned
        NumberAnimation { duration: Anim.moveMs; easing.type: Easing.OutQuad }
    }

    function updatePosition() {
        var extraOffset = 0
        if(!root.stacked && root.mediaWidget && root.mediaWidget.visible)
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
    WidgetTooltip {
        id: tip
    }

    function showTip(globalPoint, text) {
        tip.showAt(globalPoint, text)
    }
    function hideTip() {
        tip.hide()
    }

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: cardHover.hovered ? "#2A2A2A" : "transparent"

        HoverHandler {
            id: cardHover
        }

        WheelHandler {
            enabled: root.stacked
            onWheel: (event) => root.stack.cycle(event.angleDelta.y > 0 ? -1 : 1)
        }

        Row {
            id: content
            anchors.centerIn: parent
            spacing: 7
            transform: Translate { y: pageFx.offset }

            Repeater {
                model: root.shownSections
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
                                    charging: modelData.charging
                                    showGauge: DeviceIcons.hasBattery(modelData.connectionKind, modelData.percent)

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
                                                    if(DeviceIcons.hasBattery(modelData.connectionKind, modelData.percent)) {
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
