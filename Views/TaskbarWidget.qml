import QtQuick
import QtQuick.Window
import MediaFlyouts

DockableWidget {
    id: root
    readonly property int pauseAllSlot: 24
    readonly property int naturalWidth: 220 + (root.pauseAllVisible ? root.pauseAllSlot : 0)
    readonly property int naturalHeight: 40
    width: root.stacked ? root.stack.frameWidth : root.naturalWidth
    height: root.stacked ? root.stack.frameHeight : root.naturalHeight
    logName: "TaskbarWidget"
    pollActive: settings.taskbarWidgetEnabled && nowPlaying.hasMedia

    readonly property bool pauseAllVisible: settings.taskbarWidgetPauseAllEnabled
    readonly property bool showPlayAll: settings.taskbarWidgetPlayAllEnabled && nowPlaying.canResumeAll
    readonly property bool pauseAllButtonShown: root.pauseAllVisible
                && (settings.taskbarWidgetPlayAllEnabled || !nowPlaying.canResumeAll)

    property var stack: null
    readonly property bool stacked: root.stack !== null && root.stack.enabled

    readonly property bool eligible: settings.onboardingCompleted && settings.taskbarWidgetEnabled && nowPlaying.hasMedia && !root.blocked
                && settings.playerAllowed(nowPlaying.playerName)
    readonly property bool pageActive: !root.stacked || root.stack.showsMedia
    property bool pageShown: root.pageActive
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

    SlideFade {
        id: pageFx
        target: content
        direction: root.stack ? root.stack.lastStep : 1
        onFinishedLeave: root.pageShown = false
    }

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

    readonly property bool visualizerLive: root.visible && settings.taskbarVisualizerEnabled
    onVisualizerLiveChanged: root.visualizerLive ? audioCapture.start() : audioCapture.stop()

    Component.onCompleted: {
        updatePosition()
        if(root.visualizerLive)
            audioCapture.start()
    }

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
        color: hover.hovered ? root.hoverBg : "transparent"
        Behavior on color { ColorAnimation { duration: Anim.hoverMs } }

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
                if(!swiped && nowPlaying.sessionCount > 1 && Math.abs(mouse.x - pressX) > 40) {
                    root.switchSession(mouse.x < pressX ? 1 : -1)
                    swiped = true
                }
            }
            onClicked: if(!swiped) root.activated()
            onWheel: (wheel) => {
                if(root.stacked) {
                    root.stack.cycle(wheel.angleDelta.y > 0 ? -1 : 1)
                    return
                }
                if(nowPlaying.sessionCount > 1 && !root.scrollCooldown) {
                    root.switchSession(wheel.angleDelta.y > 0 ? -1 : 1)
                    root.scrollCooldown = true
                    cooldownTimer.restart()
                }
            }
        }

        Item {
            id: content
            width: root.naturalWidth
            height: root.naturalHeight
            anchors.centerIn: parent
            transform: Translate { y: pageFx.offset }

            TaskbarVisualizer {
                id: visualizer
                visible: settings.taskbarVisualizerEnabled
                accent: root.accent
                anchors.fill: parent
                anchors.margins: 3
                anchors.rightMargin: 3 + (root.pauseAllVisible ? root.pauseAllSlot : 0)
            }

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
                        color: index === nowPlaying.selectedSessionIndex ? "#4CC2FF" : root.fgMuted
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
                    color: root.artPlaceholder
                    Behavior on color { ColorAnimation { duration: Anim.moveMs } }
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

                        TextMetrics {
                            id: titleMetrics
                            font: titleText.font
                            text: nowPlaying.title
                        }

                        Text {
                            id: titleText
                            text: nowPlaying.title
                            color: root.fgPrimary
                            Behavior on color { ColorAnimation { duration: Anim.moveMs } }
                            font.pixelSize: 12
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
                        color: root.fgSecondary
                        Behavior on color { ColorAnimation { duration: Anim.moveMs } }
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
                        idleColor: root.fgControl
                        hoverColor: root.fgPrimary
                        tip: widgetTip
                        tooltip: "Previous"
                        onClicked: nowPlaying.previous()
                    }
                    WidgetButton {
                        glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                        size: 16
                        idleColor: root.fgPrimary
                        hoverColor: root.fgPrimary
                        tip: widgetTip
                        tooltip: nowPlaying.isPlaying ? "Pause" : "Play"
                        onClicked: nowPlaying.playPause()
                    }
                    WidgetButton {
                        glyph: Fluent.next
                        size: 14
                        idleColor: root.fgControl
                        hoverColor: root.fgPrimary
                        tip: widgetTip
                        tooltip: "Next"
                        onClicked: nowPlaying.next()
                    }
                }
            }

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
                idleColor: root.fgControl
                hoverColor: root.fgPrimary
                tip: widgetTip
                tooltip: root.showPlayAll ? "Play all" : "Pause all"
                onClicked: root.showPlayAll ? nowPlaying.playAll() : nowPlaying.pauseAll()
            }
        }
    }
}
