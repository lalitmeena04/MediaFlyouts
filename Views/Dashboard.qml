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
        { "icon": Fluent.speaker2,  "name": "Volume",       "desc": "Configure the volume flyout and native OSD override." },
        { "icon": Fluent.keyboard,  "name": "Lock Keys",    "desc": "Caps / Num / Scroll lock indicator flyouts." },
        { "icon": Fluent.musicNote, "name": "Taskbar Widget", "desc": "A compact now-playing widget docked on the taskbar." },
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

                // Media Flyout page
                Column {
                    visible: dash.currentIndex === 1
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: "Media flyout"
                        description: "Show the now-playing flyout on media key presses."
                        value: settings.mediaFlyoutEnabled
                        onToggled: settings.mediaFlyoutEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Up next flyout"
                        description: "Show a compact card when a track auto-advances."
                        value: settings.nextUpEnabled
                        onToggled: settings.nextUpEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Hide during fullscreen"
                        description: "Suppress flyouts while a fullscreen or exclusive app is in the foreground."
                        value: settings.disableOnFullscreen
                        onToggled: settings.disableOnFullscreen = value
                    }

                    SettingCombo {
                        width: parent.width
                        label: "Flyout position"
                        description: "Where flyouts appear on the selected monitor."
                        model: ["Top left", "Top center", "Top right", "Bottom left", "Bottom center", "Bottom right"]
                        currentIndex: settings.flyoutPosition
                        onActivated: settings.flyoutPosition = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Monitor"
                        description: "Which display flyouts are shown on."
                        model: placement.monitorNames()
                        currentIndex: settings.flyoutMonitor
                        onActivated: settings.flyoutMonitor = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: "Flyout duration"
                        description: "How long the media flyout stays on screen."
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.flyoutDuration
                        valueText: (settings.flyoutDuration / 1000).toFixed(1) + " s"
                        onMoved: settings.flyoutDuration = Math.round(value)
                    }
                }

                // Volume page
                Column {
                    visible: dash.currentIndex === 2
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: "Volume flyout"
                        description: "Show the volume flyout on volume key presses."
                        value: settings.volumeFlyoutEnabled
                        onToggled: settings.volumeFlyoutEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Override native volume OSD"
                        description: "Replace the built-in Windows volume overlay with this one."
                        value: settings.overrideNativeOsd
                        onToggled: settings.overrideNativeOsd = value
                    }
                }

                // Lock Keys page
                Column {
                    visible: dash.currentIndex === 3
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: "Lock key indicators"
                        description: "Show a flyout when Caps, Num, or Scroll lock is toggled."
                        value: settings.lockKeysEnabled
                        onToggled: settings.lockKeysEnabled = value
                    }
                }

                // Taskbar Widget page
                Column {
                    visible: dash.currentIndex === 4
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: "Taskbar widget"
                        description: "Dock a compact now-playing widget on the taskbar near the tray."
                        value: settings.taskbarWidgetEnabled
                        onToggled: settings.taskbarWidgetEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Widget transport controls"
                        description: "Show previous / play-pause / next buttons on the widget."
                        value: settings.taskbarWidgetControlsEnabled
                        onToggled: settings.taskbarWidgetControlsEnabled = value
                    }
                    SettingSlider {
                        width: parent.width
                        label: "Widget offset"
                        description: "Shift the widget left to clear third-party taskbar widgets (e.g. speed meters)."
                        from: 0
                        to: 600
                        stepSize: 10
                        value: settings.taskbarWidgetOffset
                        valueText: settings.taskbarWidgetOffset + " px"
                        onMoved: settings.taskbarWidgetOffset = Math.round(value)
                    }
                }

                // About page
                Column {
                    visible: dash.currentIndex === 5
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: "Launch on startup"
                        description: "Start MediaFlyouts automatically when you sign in."
                        value: settings.launchOnStartup
                        onToggled: settings.launchOnStartup = value
                    }
                }
            }
        }
    }
}
