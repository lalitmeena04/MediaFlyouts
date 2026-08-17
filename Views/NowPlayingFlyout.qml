import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: root
    // widen to fit the track text, clamped to a sensible range
    readonly property int contentWidth: Math.max(titleMetrics.implicitWidth, artistMetrics.implicitWidth, 80)
    width: Math.round(Math.max(300, Math.min(560, 100 + contentWidth)))
    height: 96
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
        if(!settings.nextUpEnabled || !nowPlaying.hasMedia || fullscreen.shouldBlock(settings.disableOnFullscreen))
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
        color: "#EE1E1E1E"
        border.color: "#22FFFFFF"
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

            Rectangle {
                width: 64
                height: 64
                radius: 8
                color: "#333333"
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
                    color: "#9AA0A6"
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 64 - 12
                spacing: 3

                Text {
                    text: "Now playing"
                    color: "#4CC2FF"
                    font.pixelSize: 11
                    font.bold: true
                }
                Text {
                    width: parent.width
                    text: nowPlaying.title
                    color: "white"
                    font.pixelSize: 15
                    font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: nowPlaying.artist
                    color: "#BBBBBB"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }
        }
    }
}
