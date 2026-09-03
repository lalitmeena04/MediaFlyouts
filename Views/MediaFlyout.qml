import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: root
    width: settings.compactLayout ? 330 : 380
    height: settings.compactLayout ? 96 : (settings.seekbarEnabled ? 150 : 112)
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    readonly property bool compact: settings.compactLayout
    readonly property int artSize: compact ? 52 : 76

    readonly property color accentColor: settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent

    // true once App has placed this flyout, so re-stacking animates while the first show snaps
    property bool positioned: false
    Behavior on y {
        enabled: root.positioned
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    // tracks the pinned state to detect when the user unpins while the flyout is up
    property bool prevAlwaysDisplay: settings.mediaFlyoutAlwaysDisplay

    function showFlyout() {
        if(!settings.onboardingCompleted)
            return
        if(!settings.mediaFlyoutEnabled || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.flyoutMonitor))
            return
        if(nowPlaying.hasMedia && !settings.playerAllowed(nowPlaying.playerName))
            return
        if(!settings.mediaFlyoutAlwaysDisplay)
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

    Connections {
        target: settings
        function onChanged() {
            // unpinned while the flyout is up -> resume the auto-hide countdown
            if(root.prevAlwaysDisplay && !settings.mediaFlyoutAlwaysDisplay && root.visible)
                hideTimer.restart()
            root.prevAlwaysDisplay = settings.mediaFlyoutAlwaysDisplay
        }
    }

    Timer {
        id: hideTimer
        interval: settings.flyoutDuration
        onTriggered: root.hideFlyout()
    }

    NumberAnimation {
        id: fadeIn
        target: root
        property: "opacity"
        to: 1
        duration: settings.flyoutAnimationMs
        easing.type: Anim.easingFor(settings.flyoutAnimationEasingStyle)
    }

    NumberAnimation {
        id: fadeOut
        target: root
        property: "opacity"
        to: 0
        duration: settings.flyoutAnimationMs
        easing.type: Anim.easingFor(settings.flyoutAnimationEasingStyle)
        onFinished: root.visible = false
    }

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 8
        radius: 14
        color: theme.flyout
        border.color: theme.flyoutBorder
        border.width: 1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: hideTimer.stop()
            onExited: if(!settings.mediaFlyoutAlwaysDisplay) hideTimer.restart()
            onClicked: if(!settings.mediaFlyoutAlwaysDisplay) hideTimer.restart()
        }

        // dismiss button, shown only when the flyout is pinned (always-display)
        IconLabel {
            id: closeButton
            z: 100
            visible: settings.mediaFlyoutAlwaysDisplay
            glyph: Fluent.dismiss
            size: 13
            color: closeArea.containsMouse ? theme.textPrimary : theme.textTertiary
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 9
            anchors.rightMargin: 11
            MouseArea {
                id: closeArea
                anchors.fill: parent
                anchors.margins: -6
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.hideFlyout()
            }
        }

        Row {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 14

            Rectangle {
                width: root.artSize
                height: root.artSize
                anchors.verticalCenter: parent.verticalCenter
                radius: 8
                color: theme.artPlaceholder
                clip: true

                RoundedArt {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    radius: 8
                }

                Text {
                    anchors.centerIn: parent
                    visible: !nowPlaying.hasArt
                    text: Fluent.musicNote
                    font.family: Fluent.fontFamily
                    color: theme.textTertiary
                    font.pixelSize: root.compact ? 24 : 34
                }
            }

            Column {
                width: parent.width - root.artSize - 14
                anchors.verticalCenter: parent.verticalCenter
                spacing: root.compact ? 3 : 6

                Text {
                    width: parent.width
                    text: nowPlaying.hasMedia ? nowPlaying.title : "Nothing playing"
                    color: theme.textPrimary
                    font.pixelSize: root.compact ? 14 : 16
                    font.bold: true
                    elide: Text.ElideRight
                    horizontalAlignment: settings.centerTitleArtist ? Text.AlignHCenter : Text.AlignLeft
                }

                Text {
                    width: parent.width
                    text: nowPlaying.hasMedia ? nowPlaying.artist : "Start media to see it here"
                    color: theme.textSecondary
                    font.pixelSize: root.compact ? 12 : 13
                    elide: Text.ElideRight
                    horizontalAlignment: settings.centerTitleArtist ? Text.AlignHCenter : Text.AlignLeft
                }

                Text {
                    visible: settings.playerInfoEnabled && nowPlaying.playerName.length > 0 && !root.compact
                    width: parent.width
                    text: nowPlaying.playerName
                    color: theme.textMuted
                    font.pixelSize: 11
                    elide: Text.ElideRight
                    horizontalAlignment: settings.centerTitleArtist ? Text.AlignHCenter : Text.AlignLeft
                }

                // transport controls in their own row so the seekbar can be hidden independently
                Item {
                    width: parent.width
                    height: controls.height
                    Row {
                        id: controls
                        anchors.left: settings.centerTitleArtist ? undefined : parent.left
                        anchors.horizontalCenter: settings.centerTitleArtist ? parent.horizontalCenter : undefined
                        spacing: 16

                        IconLabel {
                            visible: settings.shuffleEnabled && nowPlaying.canShuffle
                            glyph: Fluent.shuffle
                            size: root.compact ? 14 : 16
                            color: nowPlaying.shuffle ? root.accentColor : theme.textSecondary
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.toggleShuffle() }
                        }
                        IconLabel {
                            glyph: Fluent.previous
                            size: root.compact ? 16 : 18
                            color: previousArea.containsMouse ? theme.textPrimary : theme.textSecondary
                            MouseArea { id: previousArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.previous() }
                        }
                        IconLabel {
                            glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                            size: root.compact ? 18 : 20
                            color: theme.textPrimary
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.playPause() }
                        }
                        IconLabel {
                            glyph: Fluent.next
                            size: root.compact ? 16 : 18
                            color: nextArea.containsMouse ? theme.textPrimary : theme.textSecondary
                            MouseArea { id: nextArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.next() }
                        }
                        IconLabel {
                            visible: settings.repeatEnabled && nowPlaying.canRepeat
                            glyph: nowPlaying.repeatMode === 1 ? Fluent.repeatOne : Fluent.repeatAll
                            size: root.compact ? 14 : 16
                            color: nowPlaying.repeatMode !== 0 ? root.accentColor : theme.textSecondary
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.cycleRepeat() }
                        }
                    }
                }

                // seekbar (hidden in compact layout or when disabled)
                Column {
                    width: parent.width
                    visible: settings.seekbarEnabled && !root.compact
                    spacing: 4

                    Rectangle {
                        id: seekTrack
                        width: parent.width
                        height: 5
                        radius: 2.5
                        color: theme.track

                        // -1 when not scrubbing; otherwise the dragged position (0..1)
                        property real dragFraction: -1
                        readonly property real shownFraction: dragFraction >= 0 ? dragFraction : nowPlaying.progress

                        Rectangle {
                            width: seekTrack.width * seekTrack.shownFraction
                            height: parent.height
                            radius: parent.radius
                            color: root.accentColor
                            // don't animate the fill while the user is scrubbing
                            Behavior on width { enabled: seekTrack.dragFraction < 0; NumberAnimation { duration: 250 } }
                        }

                        Rectangle {
                            id: seekThumb
                            visible: nowPlaying.canSeek && settings.seekbarInteractive
                            width: (seekArea.containsMouse || seekArea.pressed) ? 13 : 10
                            height: width
                            radius: width / 2
                            color: theme.textPrimary
                            border.color: root.accentColor
                            border.width: 2
                            y: (parent.height - height) / 2
                            x: Math.max(0, Math.min(seekTrack.width - width,
                                        seekTrack.width * seekTrack.shownFraction - width / 2))
                            Behavior on width { NumberAnimation { duration: 100 } }
                        }

                        MouseArea {
                            id: seekArea
                            anchors.fill: parent
                            anchors.topMargin: -8
                            anchors.bottomMargin: -8
                            hoverEnabled: true
                            preventStealing: true
                            enabled: nowPlaying.canSeek && settings.seekbarInteractive
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                            function fractionAt(px) {
                                return Math.max(0, Math.min(1, px / seekTrack.width))
                            }

                            onEntered: hideTimer.stop()
                            onExited: if(!settings.mediaFlyoutAlwaysDisplay && !pressed) hideTimer.restart()
                            onPressed: (mouse) => { hideTimer.stop(); seekTrack.dragFraction = fractionAt(mouse.x) }
                            onPositionChanged: (mouse) => { if(pressed) seekTrack.dragFraction = fractionAt(mouse.x) }
                            onReleased: {
                                if(seekTrack.dragFraction >= 0)
                                    nowPlaying.seekToFraction(seekTrack.dragFraction)
                                seekTrack.dragFraction = -1
                                if(!settings.mediaFlyoutAlwaysDisplay)
                                    hideTimer.restart()
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 14
                        Text {
                            anchors.left: parent.left
                            text: nowPlaying.elapsed
                            color: theme.textTertiary
                            font.pixelSize: 11
                        }
                        Text {
                            anchors.right: parent.right
                            text: nowPlaying.duration
                            color: theme.textTertiary
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }
    }
}
