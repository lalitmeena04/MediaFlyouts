import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: root
    width: 320
    // grows to fit the per-app mixer when expanded
    property bool expanded: false
    // true while an app slider is being dragged, so the refresh timer doesn't rebuild the list
    property bool appDragging: false
    readonly property int sessionCount: volumeMixer.sessions.length
    height: 64 + (expanded ? (16 + Math.max(1, sessionCount) * 48) : 0)
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

    Component.onCompleted: windowEffects.setDarkFrame(root, theme.dark)

    function showFlyout() {
        if(!settings.onboardingCompleted)
            return
        if(!settings.volumeFlyoutEnabled || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.volumeFlyoutMonitor))
            return
        hideTimer.restart()
        root.visible = true
        fadeIn.restart()
    }

    function hideFlyout() {
        fadeOut.restart()
    }

    // reveal / hide the per-app mixer, keeping the flyout open while it's showing
    function toggleMixer() {
        root.expanded = !root.expanded
        if(root.expanded) {
            volumeMixer.refresh()
            hideTimer.stop()
        } else {
            hideTimer.restart()
        }
    }

    // keep the app levels fresh while the mixer is open
    Timer {
        interval: 1000
        running: root.visible && root.expanded && !root.appDragging
        repeat: true
        onTriggered: volumeMixer.refresh()
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
        onFinished: { root.visible = false; root.expanded = false }
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
            onExited: if(!root.expanded) hideTimer.restart()
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            // master volume row
            Row {
                width: parent.width
                height: 40
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
                    width: parent.width - 22 - 44 - (settings.volumeFlyoutMixerEnabled ? 32 : 0) - 24
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

                // expand toggle for the per-app mixer
                Text {
                    visible: settings.volumeFlyoutMixerEnabled
                    text: root.expanded ? "\u25B4" : "\u25BE"
                    color: chevronArea.containsMouse ? theme.textPrimary : theme.textTertiary
                    font.pixelSize: 16
                    width: 20
                    horizontalAlignment: Text.AlignHCenter
                    anchors.verticalCenter: parent.verticalCenter
                    MouseArea {
                        id: chevronArea
                        anchors.fill: parent
                        anchors.margins: -6
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.toggleMixer()
                    }
                }
            }

            Rectangle {
                visible: root.expanded
                width: parent.width
                height: 1
                color: theme.flyoutBorder
            }

            // per-application mixer
            Column {
                visible: root.expanded
                width: parent.width
                spacing: 8

                Text {
                    visible: volumeMixer.sessions.length === 0
                    text: "No apps are playing audio."
                    color: theme.textMuted
                    font.pixelSize: 12
                }

                Repeater {
                    model: volumeMixer.sessions
                    delegate: Row {
                        id: appRow
                        required property var modelData
                        // optimistic value so the handle tracks the pointer instead of the 1s refresh
                        property real shown: modelData.volume
                        width: parent.width
                        height: 40
                        spacing: 10

                        // real app icon when we have one, else a speaker glyph; the mute glyph wins when muted
                        Item {
                            width: 18
                            height: 18
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                anchors.fill: parent
                                source: modelData.icon
                                visible: modelData.icon.length > 0 && !modelData.muted
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                                mipmap: true
                            }
                            IconLabel {
                                anchors.centerIn: parent
                                visible: modelData.icon.length === 0 || modelData.muted
                                glyph: modelData.muted ? Fluent.speakerMute : Fluent.speaker2
                                size: 16
                                color: modelData.muted ? theme.textMuted : theme.accent
                            }
                            MouseArea {
                                anchors.fill: parent
                                anchors.margins: -4
                                cursorShape: Qt.PointingHandCursor
                                onClicked: volumeMixer.setSessionMute(modelData.index, !modelData.muted)
                            }
                        }

                        Column {
                            width: parent.width - 28
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 3

                            Text {
                                width: parent.width
                                text: modelData.name
                                color: theme.textSecondary
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                id: appTrack
                                width: parent.width
                                height: 5
                                radius: 2.5
                                color: theme.track

                                Rectangle {
                                    width: appTrack.width * (appRow.shown / 100)
                                    height: parent.height
                                    radius: 2.5
                                    color: modelData.muted ? theme.textMuted : theme.accent
                                }
                                Rectangle {
                                    width: 11
                                    height: 11
                                    radius: 5.5
                                    color: theme.textPrimary
                                    y: (parent.height - height) / 2
                                    x: Math.max(0, Math.min(appTrack.width - width, appTrack.width * (appRow.shown / 100) - width / 2))
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    anchors.margins: -8
                                    function setFromX(px) {
                                        const ratio = Math.max(0, Math.min(1, px / appTrack.width))
                                        appRow.shown = Math.round(ratio * 100)
                                        volumeMixer.setSessionVolume(modelData.index, appRow.shown)
                                        hideTimer.stop()
                                    }
                                    onPressed: (mouse) => { root.appDragging = true; setFromX(mouse.x) }
                                    onReleased: root.appDragging = false
                                    onPositionChanged: (mouse) => {
                                        if(pressed)
                                            setFromX(mouse.x)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
