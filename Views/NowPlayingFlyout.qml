import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: root
    readonly property bool compact: settings.nextUpStyle === 1
    // flyout accent follows the album art when enabled, else the theme accent
    readonly property color accentColor: settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent
    // widen to fit the track text, clamped to a sensible range
    readonly property int contentWidth: Math.max(titleMetrics.implicitWidth, artistMetrics.implicitWidth, 80)
    width: Math.round(Math.max(300, Math.min(560, (root.compact ? 190 : 100) + contentWidth)))
    height: root.compact ? 64 : 96
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    Text { id: titleMetrics; visible: false; text: nowPlaying.title; font.pixelSize: 15; font.bold: true }
    Text { id: artistMetrics; visible: false; text: nowPlaying.artist; font.pixelSize: 12 }

    // true once App has placed this flyout, so re-stacking animates while the first show snaps
    property bool positioned: false
    Behavior on y {
        enabled: root.positioned
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    function showFlyout() {
        if(!settings.onboardingCompleted)
            return
        if(!settings.nextUpEnabled || !nowPlaying.hasMedia || fullscreen.shouldBlock(settings.disableOnFullscreen))
            return
        if(!settings.playerAllowed(nowPlaying.playerName))
            return
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
        interval: settings.nextUpDuration
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
            onExited: hideTimer.restart()
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

        // compact pill: [note Now playing:] [art] [title / artist]
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
