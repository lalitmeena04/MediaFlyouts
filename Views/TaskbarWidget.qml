import QtQuick
import QtQuick.Window
import MediaFlyouts

// A small always-on-top media widget docked on the primary taskbar near the tray.
Window {
    id: root
    width: 220
    height: 40
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus

    // shown only when enabled, something is playing, and not blocked by fullscreen / lock screen
    visible: settings.taskbarWidgetEnabled && nowPlaying.hasMedia && !root.blocked

    // fullscreen/lock state is polled (shouldBlock has no change signal) so this can't be a binding
    property bool blocked: false

    signal activated()

    function updatePosition() {
        var p = taskbar.widgetPosition(settings.taskbarWidgetMonitor, settings.taskbarWidgetSide,
                                       settings.taskbarWidgetOffset, width, height)
        root.x = p.x
        root.y = p.y
        // keep the widget above the (possibly transparent) taskbar so it stays clickable
        if(root.visible)
            taskbar.pinAboveTaskbar(root)
    }

    Component.onCompleted: updatePosition()
    onVisibleChanged: if(visible) updatePosition()

    // reposition immediately when the offset (or other settings) change
    Connections {
        target: settings
        function onChanged() { root.updatePosition() }
    }

    // poll fullscreen/lock state and re-assert placement/z-order while media is playing,
    // even when hidden, so the widget reappears once the fullscreen app is gone
    Timer {
        interval: 500
        running: settings.taskbarWidgetEnabled && nowPlaying.hasMedia
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            root.blocked = fullscreen.shouldBlock(settings.disableOnFullscreen)
            root.updatePosition()
        }
    }

    Rectangle {
        id: card
        anchors.fill: parent
        radius: 6
        color: hover.hovered ? "#2A2A2A" : "transparent"

        HoverHandler {
            id: hover
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.activated()
        }

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Rectangle {
                width: 30
                height: 30
                radius: 5
                color: "#333333"
                clip: true
                anchors.verticalCenter: parent.verticalCenter

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
                    font.family: Fluent.fontFamily
                    color: "#4CC2FF"
                    font.pixelSize: 18
                }

                // dim + pause glyph when playback is paused
                Rectangle {
                    anchors.fill: parent
                    radius: 5
                    visible: settings.taskbarWidgetShowPauseOverlay && nowPlaying.hasMedia && !nowPlaying.isPlaying
                    color: "#99000000"
                    Text {
                        anchors.centerIn: parent
                        text: Fluent.pause
                        font.family: Fluent.fontFamily
                        color: "white"
                        font.pixelSize: 13
                    }
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0
                width: controlsRow.visible ? 78 : 132

                Item {
                    id: titleClip
                    width: parent.width
                    height: titleText.implicitHeight
                    clip: true

                    Text {
                        id: titleText
                        text: nowPlaying.title
                        color: "white"
                        font.pixelSize: 12
                        // when scrolling, size to content and slide; otherwise elide
                        property bool overflow: settings.taskbarWidgetScrollingEnabled && implicitWidth > titleClip.width
                        width: overflow ? implicitWidth : titleClip.width
                        elide: overflow ? Text.ElideNone : Text.ElideRight
                        onOverflowChanged: if(!overflow) x = 0

                        SequentialAnimation on x {
                            running: titleText.overflow
                            loops: Animation.Infinite
                            PauseAnimation { duration: 1200 }
                            NumberAnimation { from: 0; to: Math.min(0, titleClip.width - titleText.implicitWidth); duration: 3200 }
                            PauseAnimation { duration: 1200 }
                            NumberAnimation { to: 0; duration: 600 }
                        }
                    }
                }

                Text {
                    width: parent.width
                    text: nowPlaying.artist
                    color: "#AAAAAA"
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }

            Row {
                id: controlsRow
                visible: settings.taskbarWidgetControlsEnabled
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                IconLabel {
                    glyph: Fluent.previous
                    size: 14
                    color: prevArea.containsMouse ? "white" : "#C8C8C8"
                    MouseArea {
                        id: prevArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: nowPlaying.previous()
                    }
                }
                IconLabel {
                    glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                    size: 16
                    color: "white"
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: nowPlaying.playPause()
                    }
                }
                IconLabel {
                    glyph: Fluent.next
                    size: 14
                    color: nextArea.containsMouse ? "white" : "#C8C8C8"
                    MouseArea {
                        id: nextArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: nowPlaying.next()
                    }
                }
            }
        }
    }
}
