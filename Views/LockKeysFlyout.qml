import QtQuick
import QtQuick.Window
import MediaFlyouts

FlyoutBase {
    id: root
    width: 240
    height: 80
    hideInterval: settings.lockKeysDuration

    // Per-key toggle for current lock key. 0=Caps. 1=Num. 2=Scroll. 3=Insert.
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
        present()
    }

    Connections {
        target: lockKeys
        function onLockKeyToggled() {
            root.showFlyout()
        }
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
                        text: lockKeys.on ? qsTr("On") : qsTr("Off")
                        color: theme.textSecondary
                        font.pixelSize: 12
                    }
                }
            }
        }
    }
}
