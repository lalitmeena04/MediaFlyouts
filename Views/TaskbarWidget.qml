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
                && settings.playerAllowed(nowPlaying.playerName)

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

    // throttle source-switching so touchpad inertia doesn't skip several sources at once
    property bool scrollCooldown: false
    Timer {
        id: cooldownTimer
        interval: 280
        onTriggered: root.scrollCooldown = false
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
            property real pressX: 0
            property bool swiped: false
            onPressed: (mouse) => { pressX = mouse.x; swiped = false }
            onPositionChanged: (mouse) => {
                // horizontal drag / touch swipe switches media source
                if(!swiped && nowPlaying.sessionCount > 1 && Math.abs(mouse.x - pressX) > 40) {
                    nowPlaying.cycleSession(mouse.x < pressX ? 1 : -1)
                    swiped = true
                }
            }
            onClicked: if(!swiped) root.activated()
            onWheel: (wheel) => {
                // scroll wheel / touchpad scroll switches media source
                if(nowPlaying.sessionCount > 1 && !root.scrollCooldown) {
                    nowPlaying.cycleSession(wheel.angleDelta.y > 0 ? -1 : 1)
                    root.scrollCooldown = true
                    cooldownTimer.restart()
                }
            }
        }

        // source indicator dots, shown when several apps are playing
        Row {
            z: 50
            visible: nowPlaying.sessionCount > 1
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            spacing: 3
            Repeater {
                model: nowPlaying.sessionCount
                delegate: Rectangle {
                    required property int index
                    width: 4
                    height: 4
                    radius: 2
                    color: index === nowPlaying.selectedSessionIndex ? "#4CC2FF" : "#5A5A5A"
                }
            }
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

                RoundedArt {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    radius: 5
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

                    // measure the full (un-elided) title so overflow detection is stable
                    TextMetrics {
                        id: titleMetrics
                        font: titleText.font
                        text: nowPlaying.title
                    }

                    Text {
                        id: titleText
                        text: nowPlaying.title
                        color: "white"
                        font.pixelSize: 12
                        // when scrolling, size to content and slide; otherwise elide
                        property bool overflow: settings.taskbarWidgetScrollingEnabled && titleMetrics.width > titleClip.width
                        width: overflow ? titleMetrics.width : titleClip.width
                        elide: overflow ? Text.ElideNone : Text.ElideRight
                        onOverflowChanged: if(!overflow) x = 0

                        SequentialAnimation on x {
                            running: titleText.overflow
                            loops: Animation.Infinite
                            PauseAnimation { duration: 1200 }
                            NumberAnimation { from: 0; to: Math.min(0, titleClip.width - titleMetrics.width); duration: 3200 }
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
