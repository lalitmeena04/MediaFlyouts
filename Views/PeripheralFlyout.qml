import QtQuick
import QtQuick.Window
import MediaFlyouts

FlyoutBase {
    id: root

    property var alerts: []

    readonly property int cardWidth: 176
    width: Math.max(1, alerts.length) * cardWidth + 16
    height: 92
    hideInterval: settings.peripheralFlyoutDuration

    function eventEnabled(event) {
        switch(event) {
        case "connected":       return settings.peripheralFlyoutOnConnect
        case "disconnected":    return settings.peripheralFlyoutOnDisconnect
        case "lowBattery":
        case "veryLowBattery":  return settings.peripheralFlyoutOnLowBattery
        case "chargingStarted":
        case "chargingStopped": return settings.peripheralFlyoutOnCharging
        }
        return false
    }

    function alertEnabled(alert) {
        if(!settings.peripheralSelected(alert.name))
            return false
        if(alert.connectionKind === "usb" && !settings.peripheralFlyoutOnWired)
            return false
        return eventEnabled(alert.event)
    }

    function showFlyout(pending) {
        if(!settings.onboardingCompleted)
            return
        if(!pro.peripheralFlyoutEnabled || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.peripheralFlyoutMonitor))
            return

        var shown = []
        for(var i = 0; i < pending.length; i++)
            if(root.alertEnabled(pending[i]))
                shown.push(pending[i])
        if(shown.length === 0)
            return

        root.alerts = shown
        present()
    }

    Connections {
        target: peripheralAlerts
        function onAlertsRaised() {
            if(peripheralAlerts.pending.length > 0)
                root.showFlyout(peripheralAlerts.pending)
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 14
        color: theme.flyout
        border.color: theme.flyoutBorder
        border.width: 1

        PeekArea {
            flyout: root
        }

        Row {
            anchors.centerIn: parent
            spacing: 0

            Repeater {
                model: root.alerts
                delegate: PeripheralCard {
                    required property var modelData
                    alert: modelData
                }
            }
        }
    }
}
