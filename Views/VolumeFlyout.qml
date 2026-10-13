import QtQuick
import QtQuick.Window
import MediaFlyouts

FlyoutBase {
    id: root
    width: 320
    property bool expanded: false
    property bool appDragging: false
    readonly property int sessionCount: volumeMixer.sessions.length
    height: 64 + (expanded ? (16 + Math.max(1, sessionCount) * 48) : 0)
    hideInterval: settings.volumeFlyoutDuration

    function speakerGlyph() {
        if(volumeController.muted || volumeController.volume === 0)
            return Fluent.speakerMute
        if(volumeController.volume <= 50)
            return Fluent.speaker1
        return Fluent.speaker2
    }

    function setMasterVolume(fraction) {
        volumeController.setVolume(Math.round(fraction * 100))
        root.restartAutoHide()
    }

    Component.onCompleted: windowEffects.setDarkFrame(root, theme.dark)

    function showFlyout() {
        if(!settings.onboardingCompleted)
            return
        if(!settings.volumeFlyoutEnabled || !capabilities.audioAvailable || fullscreen.shouldBlock(settings.disableOnFullscreen, settings.volumeFlyoutMonitor))
            return
        present()
    }

    function afterHide() {
        root.expanded = false
    }

    function toggleMixer() {
        root.expanded = !root.expanded
        if(root.expanded) {
            volumeMixer.refresh()
            root.stopAutoHide()
        } else {
            root.restartAutoHide()
        }
    }

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

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 12
        color: theme.flyout
        border.color: theme.flyoutBorder
        border.width: 1

        PeekArea {
            flyout: root
            pinned: root.expanded
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

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

                    ScrubArea {
                        anchors.margins: -8
                        trackWidth: track.width
                        onScrubStarted: (fraction) => root.setMasterVolume(fraction)
                        onScrubMoved: (fraction) => root.setMasterVolume(fraction)
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
                        property real shown: modelData.volume
                        width: parent.width
                        height: 40
                        spacing: 10

                        function applyVolume(fraction) {
                            appRow.shown = Math.round(fraction * 100)
                            volumeMixer.setSessionVolume(appRow.modelData.index, appRow.shown)
                            root.stopAutoHide()
                        }

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
                                ScrubArea {
                                    anchors.margins: -8
                                    trackWidth: appTrack.width
                                    onScrubStarted: (fraction) => { root.appDragging = true; appRow.applyVolume(fraction) }
                                    onScrubMoved: (fraction) => appRow.applyVolume(fraction)
                                    onScrubEnded: root.appDragging = false
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
