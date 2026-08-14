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

    // true once App has placed this flyout, so re-stacking animates while the first show snaps
    property bool positioned: false
    Behavior on y {
        enabled: root.positioned
        NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
    }

    function showFlyout() {
        if(!settings.mediaFlyoutEnabled || fullscreen.shouldBlock(settings.disableOnFullscreen))
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
    }

    NumberAnimation {
        id: fadeOut
        target: root
        property: "opacity"
        to: 0
        duration: settings.flyoutAnimationMs
        onFinished: root.visible = false
    }

    Rectangle {
        id: card
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
            onExited: if(!settings.mediaFlyoutAlwaysDisplay) hideTimer.restart()
            onClicked: if(!settings.mediaFlyoutAlwaysDisplay) hideTimer.restart()
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
                color: "#333333"
                clip: true

                Image {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    fillMode: Image.PreserveAspectCrop
                    cache: false
                    asynchronous: true
                }

                Text {
                    anchors.centerIn: parent
                    visible: !nowPlaying.hasArt
                    text: Fluent.musicNote
                    font.family: Fluent.fontFamily
                    color: "#9AA0A6"
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
                    color: "white"
                    font.pixelSize: root.compact ? 14 : 16
                    font.bold: true
                    elide: Text.ElideRight
                    horizontalAlignment: settings.centerTitleArtist ? Text.AlignHCenter : Text.AlignLeft
                }

                Text {
                    width: parent.width
                    text: nowPlaying.hasMedia ? nowPlaying.artist : "Start media to see it here"
                    color: "#BBBBBB"
                    font.pixelSize: root.compact ? 12 : 13
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
                            glyph: Fluent.previous
                            size: root.compact ? 16 : 18
                            color: previousArea.containsMouse ? "white" : "#C8C8C8"
                            MouseArea { id: previousArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.previous() }
                        }
                        IconLabel {
                            glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                            size: root.compact ? 18 : 20
                            color: "white"
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.playPause() }
                        }
                        IconLabel {
                            glyph: Fluent.next
                            size: root.compact ? 16 : 18
                            color: nextArea.containsMouse ? "white" : "#C8C8C8"
                            MouseArea { id: nextArea; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: nowPlaying.next() }
                        }
                    }
                }

                // seekbar (hidden in compact layout or when disabled)
                Column {
                    width: parent.width
                    visible: settings.seekbarEnabled && !root.compact
                    spacing: 4

                    Rectangle {
                        width: parent.width
                        height: 4
                        radius: 2
                        color: "#3A3A3A"

                        Rectangle {
                            width: parent.width * nowPlaying.progress
                            height: parent.height
                            radius: 2
                            color: "#4CC2FF"
                            Behavior on width { NumberAnimation { duration: 250 } }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 14
                        Text {
                            anchors.left: parent.left
                            text: nowPlaying.elapsed
                            color: "#9AA0A6"
                            font.pixelSize: 11
                        }
                        Text {
                            anchors.right: parent.right
                            text: nowPlaying.duration
                            color: "#9AA0A6"
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }
    }
}
