import QtQuick
import QtQuick.Window
import MediaFlyouts

FlyoutBase {
    id: root
    readonly property bool compact: settings.nextUpStyle === 1
    // Flyout accent follows album art when enabled. Else use theme accent.
    readonly property color accentColor: settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent
    // Width fits track text. Clamp to range.
    readonly property int contentWidth: Math.max(titleMetrics.implicitWidth, artistMetrics.implicitWidth, 80)
    width: Math.round(Math.max(300, Math.min(560, (root.compact ? 190 : 100) + contentWidth)))
    height: root.compact ? 64 : 96
    hideInterval: settings.nextUpDuration

    Text { id: titleMetrics; visible: false; text: nowPlaying.title; font.pixelSize: 15; font.bold: true }
    Text { id: artistMetrics; visible: false; text: nowPlaying.artist; font.pixelSize: 12 }

    function showFlyout() {
        if(!settings.onboardingCompleted)
            return
        if(!settings.nextUpEnabled || !capabilities.mediaSessionAvailable || !nowPlaying.hasMedia || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.flyoutMonitor))
            return
        if(!settings.playerAllowed(nowPlaying.playerName))
            return
        present()
    }

    Connections {
        target: nowPlaying
        function onNextUpRequested() {
            root.showFlyout()
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 14
        color: theme.flyout
        border.color: theme.flyoutBorder
        border.width: 1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: root.stopAutoHide()
            onExited: root.restartAutoHide()
        }

        Row {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12
            visible: !root.compact

            Rectangle {
                width: 64
                height: 64
                radius: 8
                color: theme.artPlaceholder
                clip: true
                anchors.verticalCenter: parent.verticalCenter
                RoundedArt {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    radius: 8
                }
                IconLabel {
                    anchors.centerIn: parent
                    visible: !nowPlaying.hasArt
                    glyph: Fluent.musicNote
                    size: 26
                    color: theme.textTertiary
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 64 - 12
                spacing: 3

                Text {
                    text: "Now playing"
                    color: root.accentColor
                    font.pixelSize: 11
                    font.bold: true
                }
                Text {
                    width: parent.width
                    text: nowPlaying.title
                    color: theme.textPrimary
                    font.pixelSize: 15
                    font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: nowPlaying.artist
                    color: theme.textSecondary
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }
        }

        // Compact pill layout. Note, art, title, artist.
        Row {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10
            visible: root.compact

            Row {
                id: upNextLabel
                anchors.verticalCenter: parent.verticalCenter
                spacing: 5
                IconLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    glyph: Fluent.musicNote
                    size: 14
                    color: root.accentColor
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Now playing:"
                    color: root.accentColor
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            Rectangle {
                width: 36
                height: 36
                radius: 6
                color: theme.artPlaceholder
                clip: true
                anchors.verticalCenter: parent.verticalCenter
                RoundedArt {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    radius: 6
                }
                IconLabel {
                    anchors.centerIn: parent
                    visible: !nowPlaying.hasArt
                    glyph: Fluent.musicNote
                    size: 16
                    color: theme.textTertiary
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 36 - 10 - upNextLabel.width - 10
                spacing: 1

                Text {
                    width: parent.width
                    text: nowPlaying.title
                    color: theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: nowPlaying.artist
                    color: theme.textSecondary
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }
        }
    }
}
