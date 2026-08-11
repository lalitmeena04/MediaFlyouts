import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: root
    width: 340
    height: 96
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    function showFlyout() {
        if(fullscreen.isActive() || !nowPlaying.hasMedia)
            return
        x = Screen.virtualX + Math.round((Screen.width - width) / 2)
        y = Screen.virtualY + Screen.height - height - 48
        hideTimer.restart()
        root.visible = true
        fadeIn.restart()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

    Connections {
        target: nowPlaying
        function onNextUpRequested() {
            root.showFlyout()
        }
    }

    Timer {
        id: hideTimer
        interval: 2000
        onTriggered: root.hideFlyout()
    }

    NumberAnimation {
        id: fadeIn
        target: root
        property: "opacity"
        to: 1
        duration: 160
    }

    NumberAnimation {
        id: fadeOut
        target: root
        property: "opacity"
        to: 0
        duration: 220
        onFinished: root.visible = false
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 14
        color: "#EE1E1E1E"
        border.color: "#22FFFFFF"
        border.width: 1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: hideTimer.stop()
            onExited: hideTimer.restart()
        }

        Row {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Rectangle {
                width: 64
                height: 64
                radius: 8
                color: "#333333"
                clip: true
                anchors.verticalCenter: parent.verticalCenter
                Image {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    fillMode: Image.PreserveAspectCrop
                    cache: false
                }
                IconLabel {
                    anchors.centerIn: parent
                    visible: !nowPlaying.hasArt
                    glyph: Fluent.musicNote
                    size: 26
                    color: "#9AA0A6"
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 64 - 12
                spacing: 3

                Text {
                    text: "Up next"
                    color: "#4CC2FF"
                    font.pixelSize: 11
                    font.bold: true
                }
                Text {
                    width: parent.width
                    text: nowPlaying.title
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: nowPlaying.artist
                    color: "#BBBBBB"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }
        }
    }
}
