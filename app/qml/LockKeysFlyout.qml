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

    function showFlyout() {
        if(fullscreen.isActive())
            return
        x = Screen.virtualX + Math.round((Screen.width - width) / 2)
        y = Screen.virtualY + Math.round(Screen.height * 0.62)
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
        interval: 1200
        onTriggered: root.hideFlyout()
    }

    NumberAnimation {
        id: fadeIn
        target: root
        property: "opacity"
        to: 1
        duration: 120
    }

    NumberAnimation {
        id: fadeOut
        target: root
        property: "opacity"
        to: 0
        duration: 180
        onFinished: root.visible = false
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 12
        color: "#EE1E1E1E"
        border.color: "#22FFFFFF"
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 14

            IconLabel {
                glyph: Fluent.keyboard
                size: 26
                color: lockKeys.on ? "#4CC2FF" : "#8A8A8A"
                anchors.verticalCenter: parent.verticalCenter
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3

                Text {
                    text: lockKeys.keyName
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
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
                        color: "#C8C8C8"
                        font.pixelSize: 12
                    }
                }
            }
        }
    }
}
