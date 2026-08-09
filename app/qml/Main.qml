import QtQuick
import QtQuick.Window

Window {
    id: root
    width: 380
    height: 132
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    // place bottom-center of the primary screen with a small bottom margin
    Component.onCompleted: {
        x = Screen.virtualX + Math.round((Screen.width - width) / 2)
        y = Screen.virtualY + Screen.height - height - 48
    }

    function showFlyout() {
        hideTimer.restart()
        root.visible = true
        fadeIn.restart()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

    Connections {
        target: nowPlaying
        function onTrackUpdated() {
            if (nowPlaying.hasMedia)
                root.showFlyout()
            else
                root.hideFlyout()
        }
    }

    Timer {
        id: hideTimer
        interval: 4500
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
        id: card
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
            onClicked: hideTimer.restart()
        }

        Row {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 14

            Rectangle {
                width: 76
                height: 76
                radius: 8
                color: "#333333"
                clip: true

                Image {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    fillMode: Image.PreserveAspectCrop
                    cache: false
                    asynchronous: true
                }

                Text {
                    anchors.centerIn: parent
                    visible: !nowPlaying.hasArt
                    text: Fluent.musicNote
                    font.family: iconFontFamily
                    color: "#9AA0A6"
                    font.pixelSize: 34
                }
            }

            Column {
                width: parent.width - 76 - 14
                spacing: 6

                Text {
                    width: parent.width
                    text: nowPlaying.hasMedia ? nowPlaying.title : "Nothing playing"
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: nowPlaying.hasMedia ? nowPlaying.artist : "Start media to see it here"
                    color: "#BBBBBB"
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }

                Item { width: 1; height: 2 }

                Rectangle {
                    width: parent.width
                    height: 4
                    radius: 2
                    color: "#3A3A3A"

                    Rectangle {
                        width: parent.width * nowPlaying.progress
                        height: parent.height
                        radius: 2
                        color: "#4CC2FF"
                        Behavior on width { NumberAnimation { duration: 250 } }
                    }
                }

                Row {
                    width: parent.width
                    spacing: 10

                    Text {
                        text: nowPlaying.elapsed
                        color: "#9AA0A6"
                        font.pixelSize: 11
                    }

                    Item { width: parent.width - elapsedW.width - durationW.width - controls.width - 20; height: 1 }

                    Row {
                        id: controls
                        spacing: 12

                        IconLabel {
                            glyph: Fluent.previous
                            size: 18
                            color: previousArea.containsMouse ? "white" : "#C8C8C8"
                            MouseArea { id: previousArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.previous() }
                        }
                        IconLabel {
                            glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                            size: 20
                            color: "white"
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.playPause() }
                        }
                        IconLabel {
                            glyph: Fluent.next
                            size: 18
                            color: nextArea.containsMouse ? "white" : "#C8C8C8"
                            MouseArea { id: nextArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.next() }
                        }
                    }

                    Text {
                        text: nowPlaying.duration
                        color: "#9AA0A6"
                        font.pixelSize: 11
                    }
                }
            }
        }
    }

    // hidden metrics helpers for the time row spacer
    Text { id: elapsedW; visible: false; text: nowPlaying.elapsed; font.pixelSize: 11 }
    Text { id: durationW; visible: false; text: nowPlaying.duration; font.pixelSize: 11 }
}
