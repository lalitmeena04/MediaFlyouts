import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import MediaFlyouts

Window {
    id: root
    width: 420
    height: 520
    minimumWidth: 360
    minimumHeight: 300
    title: qsTr("Volume Mixer")
    color: theme.window
    visible: false

    function applyChrome() {
        windowEffects.setDarkFrame(root, theme.dark)
    }
    Component.onCompleted: applyChrome()
    onVisibleChanged: if(visible) applyChrome()
    property Connections _chromeConn: Connections {
        target: theme
        function onChanged() { root.applyChrome() }
    }

    function open() {
        volumeMixer.refresh()
        root.show()
        root.raise()
        root.requestActivate()
    }

    Timer {
        interval: 1000
        running: root.visible
        repeat: true
        onTriggered: volumeMixer.refresh()
    }

    Text {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        text: qsTr("Applications")
        color: theme.textPrimary
        font.pixelSize: 20
        font.bold: true
    }

    Text {
        id: emptyLabel
        visible: volumeMixer.sessions.length === 0
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.margins: 20
        anchors.topMargin: 12
        text: qsTr("No apps are playing audio right now.")
        color: theme.textMuted
        font.pixelSize: 13
    }

    ScrollView {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        anchors.topMargin: 12
        clip: true
        contentWidth: availableWidth

        Column {
            width: parent.width
            spacing: 10

            Repeater {
                model: volumeMixer.sessions
                delegate: Rectangle {
                    required property var modelData
                    width: parent.width
                    height: 66
                    radius: 10
                    color: theme.card
                    border.color: theme.border

                    Column {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 6

                        Row {
                            width: parent.width
                            spacing: 8
                            IconLabel {
                                glyph: modelData.muted ? Fluent.speakerMute : Fluent.speaker2
                                size: 16
                                color: modelData.muted ? theme.textMuted : theme.accent
                                anchors.verticalCenter: parent.verticalCenter
                                MouseArea {
                                    anchors.fill: parent
                                    anchors.margins: -6
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: volumeMixer.setSessionMute(modelData.index, !modelData.muted)
                                }
                            }
                            Text {
                                width: parent.width - 90
                                text: modelData.name
                                color: theme.textPrimary
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: modelData.volume + "%"
                                color: theme.textTertiary
                                font.pixelSize: 12
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Slider {
                            width: parent.width
                            from: 0
                            to: 100
                            value: modelData.volume
                            onMoved: volumeMixer.setSessionVolume(modelData.index, Math.round(value))
                        }
                    }
                }
            }
        }
    }

    onClosing: (event) => {
        event.accepted = false
        root.hide()
    }
}
