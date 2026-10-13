import QtQuick
import QtQuick.Window
import MediaFlyouts

FlyoutBase {
    id: root
    width: settings.compactLayout ? 330 : 380
    height: settings.compactLayout ? 96 : (settings.seekbarEnabled ? 150 : 112)

    readonly property bool compact: settings.compactLayout
    readonly property int artSize: compact ? 52 : 76

    readonly property color accentColor: settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent

    property bool prevAlwaysDisplay: settings.mediaFlyoutAlwaysDisplay

    function showFlyout() {
        if(!settings.onboardingCompleted)
            return
        if(!settings.mediaFlyoutEnabled || !capabilities.mediaSessionAvailable || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.flyoutMonitor))
            return
        if(nowPlaying.hasMedia && !settings.playerAllowed(nowPlaying.playerName))
            return
        present()
        if(settings.mediaFlyoutAlwaysDisplay)
            stopAutoHide()
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
            if(root.prevAlwaysDisplay && !settings.mediaFlyoutAlwaysDisplay && root.visible)
                root.restartAutoHide()
            root.prevAlwaysDisplay = settings.mediaFlyoutAlwaysDisplay
        }
    }

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 8
        radius: 14
        color: theme.flyout
        border.color: theme.flyoutBorder
        border.width: 1

        PeekArea {
            flyout: root
            pinned: settings.mediaFlyoutAlwaysDisplay
        }

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

                        property real dragFraction: -1
                        readonly property real shownFraction: dragFraction >= 0 ? dragFraction : nowPlaying.progress

                        Rectangle {
                            width: seekTrack.width * seekTrack.shownFraction
                            height: parent.height
                            radius: parent.radius
                            color: root.accentColor
                            Behavior on width { enabled: seekTrack.dragFraction < 0; NumberAnimation { duration: Anim.fillMs } }
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
                            Behavior on width { NumberAnimation { duration: Anim.hoverMs } }
                        }

                        ScrubArea {
                            id: seekArea
                            anchors.topMargin: -8
                            anchors.bottomMargin: -8
                            trackWidth: seekTrack.width
                            hoverEnabled: true
                            preventStealing: true
                            enabled: nowPlaying.canSeek && settings.seekbarInteractive
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                            onEntered: root.stopAutoHide()
                            onExited: if(!settings.mediaFlyoutAlwaysDisplay && !pressed) root.restartAutoHide()
                            onScrubStarted: (fraction) => { root.stopAutoHide(); seekTrack.dragFraction = fraction }
                            onScrubMoved: (fraction) => seekTrack.dragFraction = fraction
                            onScrubEnded: (fraction) => {
                                nowPlaying.seekToFraction(fraction)
                                seekTrack.dragFraction = -1
                                if(!settings.mediaFlyoutAlwaysDisplay)
                                    root.restartAutoHide()
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
