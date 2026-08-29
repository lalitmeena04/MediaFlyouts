import QtQuick
import QtQuick.Window
import MediaFlyouts

// Shows peripheral events (connect / disconnect / battery) as a row of cards. Several
// devices reported in the same snapshot appear side by side in one window rather than as
// separate flyouts fighting over the same screen edge.
Window {
    id: root

    property var alerts: []

    readonly property int cardWidth: 176
    width: Math.max(1, alerts.length) * cardWidth + 16
    height: 92
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    // true once App has placed this flyout, so re-stacking animates while the first show snaps
    property bool positioned: false
    Behavior on y {
        enabled: root.positioned
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    // Events the user has switched off never reach the screen.
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

    function showFlyout(pending) {
        if(!settings.onboardingCompleted)
            return
        if(!settings.peripheralFlyoutEnabled || fullscreen.shouldBlock(settings.disableOnFullscreen))
            return

        var shown = []
        for(var i = 0; i < pending.length; i++)
            if(root.eventEnabled(pending[i].event))
                shown.push(pending[i])
        if(shown.length === 0)
            return

        root.alerts = shown
        hideTimer.restart()
        root.visible = true
        fadeIn.restart()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

    Connections {
        target: peripheralAlerts
        function onAlertsRaised() {
            if(peripheralAlerts.pending.length > 0)
                root.showFlyout(peripheralAlerts.pending)
        }
    }

    Timer {
        id: hideTimer
        interval: settings.peripheralFlyoutDuration
        onTriggered: root.hideFlyout()
    }

    NumberAnimation {
        id: fadeIn
        target: root
        property: "opacity"
        to: 1
        duration: settings.flyoutAnimationMs
        easing.type: Anim.easingFor(settings.flyoutAnimationEasingStyle)
    }

    NumberAnimation {
        id: fadeOut
        target: root
        property: "opacity"
        to: 0
        duration: settings.flyoutAnimationMs
        easing.type: Anim.easingFor(settings.flyoutAnimationEasingStyle)
        onFinished: root.visible = false
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 14
        color: theme.flyout
        border.color: theme.flyoutBorder
        border.width: 1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: hideTimer.stop()
            onExited: hideTimer.restart()
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
