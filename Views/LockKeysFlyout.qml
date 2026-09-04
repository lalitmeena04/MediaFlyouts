import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: root
    width: 240
    height: 80
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

    // per-key toggle for the currently reported lock key (keyType: 0=Caps,1=Num,2=Scroll,3=Insert)
    function keyEnabled() {
        switch(lockKeys.keyType) {
        case 0: return settings.lockKeysCapsEnabled
        case 1: return settings.lockKeysNumEnabled
        case 2: return settings.lockKeysScrollEnabled
        case 3: return settings.lockKeysInsertEnabled
        }
        return true
    }

    function showFlyout() {
        if(!settings.onboardingCompleted)
            return
        if(!settings.lockKeysEnabled || !capabilities.lockKeysAvailable || !keyEnabled() || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.flyoutMonitor))
            return
        hideTimer.restart()
        root.visible = true
        fadeIn.restart()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

    Connections {
        target: lockKeys
        function onLockKeyToggled() {
            root.showFlyout()
        }
    }

    Timer {
        id: hideTimer
        interval: settings.lockKeysDuration
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
        radius: 12
        color: theme.flyout
        border.color: theme.flyoutBorder
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 14

            IconLabel {
                glyph: Fluent.keyboard
                size: 26
                color: lockKeys.on ? theme.accent : theme.textMuted
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3

                Text {
                    text: lockKeys.keyName
                    color: theme.textPrimary
                    font.pixelSize: 15
                    font.bold: settings.lockKeysBoldUi
                }

                Row {
                    spacing: 6
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: lockKeys.on ? "#3FB950" : "#6E6E6E"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: lockKeys.on ? "On" : "Off"
                        color: theme.textSecondary
                        font.pixelSize: 12
                    }
                }
            }
        }
    }
}
