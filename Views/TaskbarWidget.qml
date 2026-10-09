import QtQuick
import QtQuick.Window
import MediaFlyouts

// Small always-on-top media widget. Docks near taskbar tray.
DockableWidget {
    id: root
    // Pause-all button adds one slot to the right of the visualizer.
    readonly property int pauseAllSlot: 24
    // Size of the content. Stack layout shares one frame between widgets.
    readonly property int naturalWidth: 220 + (root.pauseAllVisible ? root.pauseAllSlot : 0)
    readonly property int naturalHeight: 40
    width: root.stacked ? root.stack.frameWidth : root.naturalWidth
    height: root.stacked ? root.stack.frameHeight : root.naturalHeight
    logName: "TaskbarWidget"
    // Keep polling while media plays. Reappear after fullscreen.
    pollActive: settings.taskbarWidgetEnabled && nowPlaying.hasMedia

    readonly property bool pauseAllVisible: settings.taskbarWidgetPauseAllEnabled
    // Button flips to play-all after pause-all, when the follow-up option is on.
    readonly property bool showPlayAll: settings.taskbarWidgetPlayAllEnabled && nowPlaying.canResumeAll
    // Without play-all the button has no job once everything is paused. Slot stays reserved.
    readonly property bool pauseAllButtonShown: root.pauseAllVisible
                && (settings.taskbarWidgetPlayAllEnabled || !nowPlaying.canResumeAll)
    onWidthChanged: updatePosition()

    // WidgetStack from App.qml. Null means side-by-side layout.
    property var stack: null
    readonly property bool stacked: root.stack !== null && root.stack.enabled

    // True when the widget has something to show. Stack layout can still hide it.
    readonly property bool eligible: settings.onboardingCompleted && settings.taskbarWidgetEnabled && nowPlaying.hasMedia && !root.blocked
                && settings.playerAllowed(nowPlaying.playerName)
    // True on the media stack page. Always true side by side.
    readonly property bool pageActive: !root.stacked || root.stack.showsMedia
    // Lags pageActive while the page slides out.
    property bool pageShown: root.pageActive
    // Show after onboarding, when enabled, media plays, not blocked, and on the active stack page.
    visible: root.eligible && root.pageShown

    onPageActiveChanged: {
        if(root.pageActive) {
            root.pageShown = true
            if(root.stacked)
                pageFx.enter()
            else
                pageFx.reset()
        } else {
            pageFx.leave()
        }
    }

    // Stack page slides along the scroll direction.
    SlideFade {
        id: pageFx
        target: content
        direction: root.stack ? root.stack.lastStep : 1
        onFinishedLeave: root.pageShown = false
    }

    // Media source change slides the info row sideways.
    property int sessionStep: 1
    readonly property int sessionIndex: nowPlaying.selectedSessionIndex
    onSessionIndexChanged: if(root.visible) sessionFx.enter()
    SlideFade {
        id: sessionFx
        target: mediaRow
        distance: 24
        direction: root.sessionStep
    }
    function switchSession(step) {
        root.sessionStep = step
        nowPlaying.cycleSession(step)
    }

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

        // Covers the whole frame. Content buttons sit above it.
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            property real pressX: 0
            property bool swiped: false
            onPressed: (mouse) => { pressX = mouse.x; swiped = false }
            onPositionChanged: (mouse) => {
                // Horizontal drag or touch swipe switches media source.
                if(!swiped && nowPlaying.sessionCount > 1 && Math.abs(mouse.x - pressX) > 40) {
                    root.switchSession(mouse.x < pressX ? 1 : -1)
                    swiped = true
                }
            }
            onClicked: if(!swiped) root.activated()
            onWheel: (wheel) => {
                // Stack layout: wheel pages between widgets.
                if(root.stacked) {
                    root.stack.cycle(wheel.angleDelta.y > 0 ? -1 : 1)
                    return
                }
                // Scroll wheel or touchpad scroll switches media source.
                if(nowPlaying.sessionCount > 1 && !root.scrollCooldown) {
                    root.switchSession(wheel.angleDelta.y > 0 ? -1 : 1)
                    root.scrollCooldown = true
                    cooldownTimer.restart()
                }
            }
        }

        // Natural-size content. Centers in the shared stack frame.
        Item {
            id: content
            width: root.naturalWidth
            height: root.naturalHeight
            anchors.centerIn: parent
            transform: Translate { y: pageFx.offset }

            // Spectrum visualizer behind media info. Style is user-selectable.
            // It stops before the pause-all slot.
            TaskbarVisualizer {
                id: visualizer
                visible: settings.taskbarVisualizerEnabled
                anchors.fill: parent
                anchors.margins: 3
                anchors.rightMargin: 3 + (root.pauseAllVisible ? root.pauseAllSlot : 0)
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
                        Behavior on color { ColorAnimation { duration: Anim.moveMs } }
                    }
                }
            }

            Row {
                id: mediaRow
                anchors.left: parent.left
                anchors.leftMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8
                transform: Translate { x: sessionFx.offset }

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
                opacity: root.pauseAllButtonShown ? 1 : 0
                enabled: root.pauseAllButtonShown
                Behavior on opacity { NumberAnimation { duration: Anim.hoverMs } }
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
}
