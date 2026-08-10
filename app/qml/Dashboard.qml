import QtQuick
import QtQuick.Window
import MediaFlyouts

Window {
    id: dash
    width: 920
    height: 620
    minimumWidth: 760
    minimumHeight: 480
    visible: false
    title: "MediaFlyouts"
    color: "#1B1B1B"

    property int currentIndex: 0
    readonly property var pages: [
        { "icon": Fluent.musicNote, "name": "Home",         "desc": "Overview of MediaFlyouts and what's currently playing." },
        { "icon": Fluent.play,      "name": "Media Flyout", "desc": "Configure the now-playing flyout that appears on media keys." },
        { "icon": Fluent.speaker2,  "name": "Volume",       "desc": "Volume flyout and per-app mixer (coming soon)." },
        { "icon": Fluent.keyboard,  "name": "Lock Keys",    "desc": "Caps / Num / Scroll lock indicators (coming soon)." },
        { "icon": Fluent.settings,  "name": "About",        "desc": "MediaFlyouts — a Fluent-style media overlay built with Qt 6." }
    ]

    // hide to tray instead of quitting the app
    onClosing: (event) => {
        event.accepted = false
        dash.hide()
    }

    Row {
        anchors.fill: parent

        Rectangle {
            id: sidebar
            width: 240
            height: parent.height
            color: "#151515"

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 8

                Row {
                    spacing: 12
                    bottomPadding: 12
                    IconLabel {
                        glyph: Fluent.musicNote
                        size: 24
                        color: "#4CC2FF"
                    }
                    Text {
                        text: "MediaFlyouts"
                        color: "white"
                        font.pixelSize: 18
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Repeater {
                    model: dash.pages

                    delegate: Rectangle {
                        required property int index
                        required property var modelData

                        width: sidebar.width - 36
                        height: 40
                        radius: 8
                        color: dash.currentIndex === index ? "#2A2A2A" : (navMouse.containsMouse ? "#202020" : "transparent")

                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            spacing: 12
                            IconLabel {
                                glyph: modelData.icon
                                size: 18
                                color: dash.currentIndex === index ? "#4CC2FF" : "#C8C8C8"
                            }
                            Text {
                                text: modelData.name
                                color: "white"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            id: navMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: dash.currentIndex = index
                        }
                    }
                }
            }
        }

        Rectangle {
            width: dash.width - sidebar.width
            height: parent.height
            color: "#1B1B1B"

            Column {
                anchors.fill: parent
                anchors.margins: 32
                spacing: 16

                Text {
                    text: dash.pages[dash.currentIndex].name
                    color: "white"
                    font.pixelSize: 26
                    font.bold: true
                }

                Text {
                    width: parent.width
                    text: dash.pages[dash.currentIndex].desc
                    color: "#B0B0B0"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    visible: dash.currentIndex === 0
                    width: parent.width
                    height: 96
                    radius: 12
                    color: "#232323"
                    border.color: "#2E2E2E"

                    Row {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        Rectangle {
                            width: 64
                            height: 64
                            radius: 8
                            color: "#333333"
                            clip: true
                            Image {
                                anchors.fill: parent
                                source: nowPlaying.artSource
                                visible: nowPlaying.hasArt
                                fillMode: Image.PreserveAspectCrop
                                cache: false
                            }
                            IconLabel {
                                anchors.centerIn: parent
                                visible: !nowPlaying.hasArt
                                glyph: Fluent.musicNote
                                size: 28
                                color: "#9AA0A6"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4
                            Text {
                                text: nowPlaying.hasMedia ? nowPlaying.title : "Nothing playing"
                                color: "white"
                                font.pixelSize: 16
                                font.bold: true
                            }
                            Text {
                                text: nowPlaying.hasMedia ? nowPlaying.artist : "Start media to see it here"
                                color: "#B0B0B0"
                                font.pixelSize: 13
                            }
                        }
                    }
                }
            }
        }
    }
}
