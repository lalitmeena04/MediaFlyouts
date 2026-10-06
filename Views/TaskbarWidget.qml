import QtQuick
import QtQuick.Window
import MediaFlyouts

// Small always-on-top media widget. Docks near taskbar tray.
DockableWidget {
    id: root
    // Pause-all button adds one slot to the right of the visualizer.
    readonly property int pauseAllSlot: 24
    width: 220 + (root.pauseAllVisible ? root.pauseAllSlot : 0)
    height: 40
    logName: "TaskbarWidget"
    // Keep polling while media plays. Reappear after fullscreen.
    pollActive: settings.taskbarWidgetEnabled && nowPlaying.hasMedia

    readonly property bool pauseAllVisible: settings.taskbarWidgetPauseAllEnabled
    // Button flips to play-all after pause-all, when the follow-up option is on.
    readonly property bool showPlayAll: settings.taskbarWidgetPlayAllEnabled && nowPlaying.canResumeAll
    onWidthChanged: updatePosition()

    // Show after onboarding, when enabled, media plays, and not blocked.
    visible: settings.onboardingCompleted && settings.taskbarWidgetEnabled && nowPlaying.hasMedia && !root.blocked
                && settings.playerAllowed(nowPlaying.playerName)

    onVisibleChanged: {
        log.debug("TaskbarWidget.visible=" + visible
            + " onboardingCompleted=" + settings.onboardingCompleted
            + " enabled=" + settings.taskbarWidgetEnabled
            + " hasMedia=" + nowPlaying.hasMedia
            + " playerAllowed=" + settings.playerAllowed(nowPlaying.playerName)
            + " blocked=" + root.blocked
            + " monitor=" + settings.taskbarWidgetMonitor)
        if(visible)
            updatePosition()
    }

    signal activated()

    // Capture system audio only when visualizer is visible.
    readonly property bool visualizerLive: root.visible && settings.taskbarVisualizerEnabled
    onVisualizerLiveChanged: root.visualizerLive ? audioCapture.start() : audioCapture.stop()

    // Place widget and apply initial capture state.
    Component.onCompleted: {
        updatePosition()
        if(root.visualizerLive)
            audioCapture.start()
    }

    // Throttle source switching from touchpad inertia.
    property bool scrollCooldown: false
    Timer {
        id: cooldownTimer
        interval: 280
        onTriggered: root.scrollCooldown = false
    }

    WidgetTooltip {
        id: widgetTip
    }

    Rectangle {
        id: card
        anchors.fill: parent
        radius: 6
        color: hover.hovered ? "#2A2A2A" : "transparent"

        HoverHandler {
            id: hover
        }

        // Spectrum visualizer behind media info. Style is user-selectable.
        // It stops before the pause-all slot.
        TaskbarVisualizer {
            id: visualizer
            visible: settings.taskbarVisualizerEnabled
            anchors.fill: parent
            anchors.margins: 3
            anchors.rightMargin: 3 + (root.pauseAllVisible ? root.pauseAllSlot : 0)
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            property real pressX: 0
            property bool swiped: false
            onPressed: (mouse) => { pressX = mouse.x; swiped = false }
            onPositionChanged: (mouse) => {
                // Horizontal drag or touch swipe switches media source.
                if(!swiped && nowPlaying.sessionCount > 1 && Math.abs(mouse.x - pressX) > 40) {
                    nowPlaying.cycleSession(mouse.x < pressX ? 1 : -1)
                    swiped = true
                }
            }
            onClicked: if(!swiped) root.activated()
            onWheel: (wheel) => {
                // Scroll wheel or touchpad scroll switches media source.
                if(nowPlaying.sessionCount > 1 && !root.scrollCooldown) {
                    nowPlaying.cycleSession(wheel.angleDelta.y > 0 ? -1 : 1)
                    root.scrollCooldown = true
                    cooldownTimer.restart()
                }
            }
        }

        // Source indicator dots. Show for multiple apps.
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

                // Dim overlay and pause glyph for paused playback.
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

                    // Measure full title. Overflow detection stays stable.
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
                        // Scrolling mode sizes to content. Else elide.
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

                WidgetButton {
                    glyph: Fluent.previous
                    size: 14
                    tip: widgetTip
                    tooltip: "Previous"
                    onClicked: nowPlaying.previous()
                }
                WidgetButton {
                    glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                    size: 16
                    idleColor: "white"
                    tip: widgetTip
                    tooltip: nowPlaying.isPlaying ? "Pause" : "Play"
                    onClicked: nowPlaying.playPause()
                }
                WidgetButton {
                    glyph: Fluent.next
                    size: 14
                    tip: widgetTip
                    tooltip: "Next"
                    onClicked: nowPlaying.next()
                }
            }
        }

        // Single pause-all / play-all button. Sits right of the visualizer boundary.
        WidgetButton {
            id: pauseAllButton
            visible: root.pauseAllVisible
            anchors.right: parent.right
            anchors.rightMargin: 3
            anchors.verticalCenter: parent.verticalCenter
            width: root.pauseAllSlot
            glyph: root.showPlayAll ? Fluent.playCircle : Fluent.pauseCircle
            size: 16
            tip: widgetTip
            tooltip: root.showPlayAll ? "Play all" : "Pause all"
            onClicked: root.showPlayAll ? nowPlaying.playAll() : nowPlaying.pauseAll()
        }
    }
}
