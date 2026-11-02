import QtQuick
import QtQuick.Window
import MediaFlyouts

FlyoutBase {
    id: root

    property bool plugged: false

    width: 264
    height: 92
    hideInterval: settings.chargerFlyoutDuration

    function eventEnabled(pluggedIn) {
        return pluggedIn ? settings.chargerFlyoutOnPlug : settings.chargerFlyoutOnUnplug
    }

    function showFlyout(pluggedIn) {
        if(!settings.onboardingCompleted)
            return
        if(!settings.chargerFlyoutEnabled || !capabilities.chargerAvailable || !eventEnabled(pluggedIn)
            || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.chargerFlyoutMonitor))
            return
        root.plugged = pluggedIn
        present()
    }

    Connections {
        target: powerStatus
        function onChargerChanged(pluggedIn) {
            root.showFlyout(pluggedIn)
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
            spacing: 12

            BatteryRing {
                anchors.verticalCenter: parent.verticalCenter
                width: 44
                height: 44
                thickness: 4
                percent: powerStatus.percent

                IconLabel {
                    anchors.centerIn: parent
                    glyph: Fluent.plugConnected
                    size: 18
                    color: root.plugged ? theme.accent : theme.textMuted
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2

                Text {
                    width: 176
                    text: root.plugged ? qsTr("Charger connected") : qsTr("Charger disconnected")
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    width: 176
                    text: root.plugged ? qsTr("Plugged in") : qsTr("On battery")
                    color: theme.textMuted
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
                Text {
                    visible: powerStatus.percent >= 0
                    width: 176
                    text: powerStatus.bucket + "  \u00B7  " + powerStatus.percent + "%"
                    color: theme.textPrimary
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }
        }
    }
}
