import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: root
    width: 320
    height: 64
    visible: false
    opacity: 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    // true once App has placed this flyout, so re-stacking animates while the first show snaps
    property bool positioned: false
    Behavior on y {
        enabled: root.positioned
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    function speakerGlyph() {
        if(volumeController.muted || volumeController.volume === 0)
            return Fluent.speakerMute
        if(volumeController.volume <= 50)
            return Fluent.speaker1
        return Fluent.speaker2
    }

    function showFlyout() {
        if(!settings.volumeFlyoutEnabled || fullscreen.shouldBlock(settings.disableOnFullscreen))
            return
        hideTimer.restart()
        root.visible = true
        fadeIn.restart()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

    Connections {
        target: volumeController
        function onVolumeChangedExternally() {
            root.showFlyout()
        }
    }

    Timer {
        id: hideTimer
        interval: settings.volumeFlyoutDuration
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
        radius: 12
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

            IconLabel {
                glyph: root.speakerGlyph()
                size: 22
                color: theme.textPrimary
                anchors.verticalCenter: parent.verticalCenter
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: volumeController.toggleMute()
                }
            }

            Rectangle {
                id: track
                width: parent.width - 22 - 44 - 24
                height: 6
                radius: 3
                color: theme.track
                anchors.verticalCenter: parent.verticalCenter

                Rectangle {
                    width: track.width * (volumeController.volume / 100)
                    height: parent.height
                    radius: 3
                    color: volumeController.muted ? theme.textMuted : theme.accent
                }

                Rectangle {
                    width: 14
                    height: 14
                    radius: 7
                    color: theme.textPrimary
                    y: (parent.height - height) / 2
                    x: Math.max(0, Math.min(track.width - width, track.width * (volumeController.volume / 100) - width / 2))
                }

                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -8
                    function setFromX(px) {
                        const ratio = Math.max(0, Math.min(1, px / track.width))
                        volumeController.setVolume(Math.round(ratio * 100))
                        hideTimer.restart()
                    }
                    onPressed: (mouse) => setFromX(mouse.x)
                    onPositionChanged: (mouse) => {
                        if(pressed)
                            setFromX(mouse.x)
                    }
                }
            }

            Text {
                text: volumeController.volume + "%"
                color: theme.textSecondary
                font.pixelSize: 13
                width: 44
                horizontalAlignment: Text.AlignRight
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
