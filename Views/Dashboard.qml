import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import QtQuick.Dialogs
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

            Text {
                id: pageTitle
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 32
                text: dash.pages[dash.currentIndex].name
                color: "white"
                font.pixelSize: 26
                font.bold: true
            }

            Text {
                id: pageDesc
                anchors.top: pageTitle.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                anchors.topMargin: 12
                text: dash.pages[dash.currentIndex].desc
                color: "#B0B0B0"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }

            // scrollable so long pages (e.g. the media flyout options) aren't cut off
            ScrollView {
                id: pageScroll
                anchors.top: pageDesc.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                anchors.topMargin: 16
                anchors.bottomMargin: 32
                clip: true
                contentWidth: availableWidth

                Column {
                    width: pageScroll.availableWidth
                    spacing: 16

                    Column {
                        id: homePage
                        visible: dash.currentIndex === 0
                        width: parent.width
                        spacing: 16

                        // live feature state, kept reactive by reading the settings properties directly
                        function featureOn(key) {
                            switch(key) {
                            case "media": return settings.mediaFlyoutEnabled
                            case "volume": return settings.volumeFlyoutEnabled
                            case "lock": return settings.lockKeysEnabled
                            case "widget": return settings.taskbarWidgetEnabled
                            case "nextup": return settings.nextUpEnabled
                            case "osd": return settings.overrideNativeOsd
                            }
                            return false
                        }

                        // Now playing hero card
                        Rectangle {
                            width: parent.width
                            height: 176
                            radius: 12
                            color: "#232323"
                            border.color: "#2E2E2E"

                            Row {
                                anchors.fill: parent
                                anchors.margins: 18
                                spacing: 18

                                Rectangle {
                                    width: 108
                                    height: 108
                                    radius: 10
                                    color: "#333333"
                                    clip: true
                                    anchors.verticalCenter: parent.verticalCenter
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
                                        size: 44
                                        color: "#9AA0A6"
                                    }
                                    Rectangle {
                                        anchors.fill: parent
                                        radius: 10
                                        visible: nowPlaying.hasMedia && !nowPlaying.isPlaying
                                        color: "#88000000"
                                        IconLabel {
                                            anchors.centerIn: parent
                                            glyph: Fluent.pause
                                            size: 24
                                            color: "white"
                                        }
                                    }
                                }

                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width - 108 - 18
                                    spacing: 6

                                    Text {
                                        width: parent.width
                                        text: nowPlaying.hasMedia ? nowPlaying.title : "Nothing playing"
                                        color: "white"
                                        font.pixelSize: 20
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        width: parent.width
                                        text: nowPlaying.hasMedia ? nowPlaying.artist : "Start media in any app to control it here"
                                        color: "#B0B0B0"
                                        font.pixelSize: 14
                                        elide: Text.ElideRight
                                    }

                                    Rectangle {
                                        visible: nowPlaying.hasMedia && nowPlaying.playerName.length > 0
                                        radius: 4
                                        color: "#26333D"
                                        height: playerText.implicitHeight + 6
                                        width: playerText.implicitWidth + 14
                                        Text {
                                            id: playerText
                                            anchors.centerIn: parent
                                            text: nowPlaying.playerName
                                            color: "#7FD6FF"
                                            font.pixelSize: 11
                                        }
                                    }

                                    // progress bar + times
                                    Column {
                                        width: parent.width
                                        spacing: 4
                                        topPadding: 6
                                        visible: nowPlaying.hasMedia

                                        Rectangle {
                                            width: parent.width
                                            height: 4
                                            radius: 2
                                            color: "#3A3A3A"
                                            Rectangle {
                                                width: parent.width * Math.max(0, Math.min(1, nowPlaying.progress))
                                                height: parent.height
                                                radius: 2
                                                color: "#4CC2FF"
                                            }
                                        }
                                        Item {
                                            width: parent.width
                                            height: elapsedText.implicitHeight
                                            Text {
                                                id: elapsedText
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

                                    // transport controls
                                    Row {
                                        spacing: 22
                                        topPadding: 8
                                        visible: nowPlaying.hasMedia
                                        IconLabel {
                                            glyph: Fluent.previous
                                            size: 18
                                            color: prevMa.containsMouse ? "white" : "#C8C8C8"
                                            MouseArea {
                                                id: prevMa
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: nowPlaying.previous()
                                            }
                                        }
                                        IconLabel {
                                            glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                                            size: 22
                                            color: playMa.containsMouse ? "white" : "#4CC2FF"
                                            MouseArea {
                                                id: playMa
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: nowPlaying.playPause()
                                            }
                                        }
                                        IconLabel {
                                            glyph: Fluent.next
                                            size: 18
                                            color: nextMa.containsMouse ? "white" : "#C8C8C8"
                                            MouseArea {
                                                id: nextMa
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: nowPlaying.next()
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            text: "At a glance"
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                            topPadding: 4
                        }

                        // feature status cards; click to jump to that section
                        Flow {
                            width: parent.width
                            spacing: 12

                            Repeater {
                                model: [
                                    { "name": "Media flyout", "icon": Fluent.play, "key": "media", "page": 1 },
                                    { "name": "Volume flyout", "icon": Fluent.speaker2, "key": "volume", "page": 2 },
                                    { "name": "Lock keys", "icon": Fluent.keyboard, "key": "lock", "page": 3 },
                                    { "name": "Taskbar widget", "icon": Fluent.musicNote, "key": "widget", "page": 4 },
                                    { "name": "Up next", "icon": Fluent.next, "key": "nextup", "page": 1 },
                                    { "name": "Native OSD override", "icon": Fluent.speaker2, "key": "osd", "page": 2 }
                                ]
                                delegate: Rectangle {
                                    required property var modelData
                                    width: (homePage.width - 24) / 3
                                    height: 76
                                    radius: 10
                                    color: cardMa.containsMouse ? "#282828" : "#232323"
                                    border.color: "#2E2E2E"

                                    Row {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 10
                                        IconLabel {
                                            anchors.verticalCenter: parent.verticalCenter
                                            glyph: modelData.icon
                                            size: 22
                                            color: homePage.featureOn(modelData.key) ? "#4CC2FF" : "#6A6A6A"
                                        }
                                        Column {
                                            anchors.verticalCenter: parent.verticalCenter
                                            spacing: 4
                                            width: parent.width - 32
                                            Text {
                                                width: parent.width
                                                text: modelData.name
                                                color: "white"
                                                font.pixelSize: 13
                                                font.bold: true
                                                elide: Text.ElideRight
                                            }
                                            Text {
                                                text: homePage.featureOn(modelData.key) ? "On" : "Off"
                                                color: homePage.featureOn(modelData.key) ? "#6FCF7F" : "#8A8A8A"
                                                font.pixelSize: 11
                                            }
                                        }
                                    }
                                    MouseArea {
                                        id: cardMa
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: dash.currentIndex = modelData.page
                                    }
                                }
                            }
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "Tip: press a media or volume key, or click the tray icon, to preview the flyouts."
                            color: "#7A7A7A"
                            font.pixelSize: 12
                            topPadding: 4
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
                        label: "Seekbar"
                        description: "Show the progress bar and elapsed / total time."
                        value: settings.seekbarEnabled
                        onToggled: settings.seekbarEnabled = value
                    }
                    SettingRow {
                        // sub-option, shown only when the seekbar itself is enabled
                        visible: settings.seekbarEnabled
                        x: 16
                        width: parent.width - 16
                        label: "Interactive seekbar"
                        description: "Allow clicking and dragging the seekbar to seek."
                        value: settings.seekbarInteractive
                        onToggled: settings.seekbarInteractive = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Compact layout"
                        description: "Use a smaller, denser media flyout."
                        value: settings.compactLayout
                        onToggled: settings.compactLayout = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Center title and artist"
                        description: "Center-align the track title and artist text."
                        value: settings.centerTitleArtist
                        onToggled: settings.centerTitleArtist = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Always display"
                        description: "Keep the media flyout on screen instead of auto-hiding."
                        value: settings.mediaFlyoutAlwaysDisplay
                        onToggled: settings.mediaFlyoutAlwaysDisplay = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Show player name"
                        description: "Show the name of the app playing the media."
                        value: settings.playerInfoEnabled
                        onToggled: settings.playerInfoEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Repeat button"
                        description: "Show a repeat control on the media flyout."
                        value: settings.repeatEnabled
                        onToggled: settings.repeatEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Shuffle button"
                        description: "Show a shuffle control on the media flyout."
                        value: settings.shuffleEnabled
                        onToggled: settings.shuffleEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Up next flyout"
                        description: "Show a compact card with the next track when one auto-advances."
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
                        description: "Which side of the monitor flyouts appear on, or docked above the taskbar widget."
                        model: ["Top left", "Top center", "Top right", "Bottom left", "Bottom center", "Bottom right", "Above taskbar widget"]
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
                    SettingCombo {
                        width: parent.width
                        label: "Animation speed"
                        description: "Flyout fade-in / fade-out speed."
                        model: ["Off", "0.5x", "1x", "1.5x", "2x", "3x"]
                        currentIndex: settings.flyoutAnimationSpeed
                        onActivated: settings.flyoutAnimationSpeed = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Animation easing"
                        description: "The easing curve used for flyout animations."
                        model: ["Linear", "Sine", "Quadratic", "Cubic"]
                        currentIndex: settings.flyoutAnimationEasingStyle
                        onActivated: settings.flyoutAnimationEasingStyle = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: "Up next duration"
                        description: "How long the up-next flyout stays on screen."
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.nextUpDuration
                        valueText: (settings.nextUpDuration / 1000).toFixed(1) + " s"
                        onMoved: settings.nextUpDuration = Math.round(value)
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
                    SettingSlider {
                        width: parent.width
                        label: "Volume flyout duration"
                        description: "How long the volume flyout stays on screen."
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.volumeFlyoutDuration
                        valueText: (settings.volumeFlyoutDuration / 1000).toFixed(1) + " s"
                        onMoved: settings.volumeFlyoutDuration = Math.round(value)
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
                    SettingRow {
                        width: parent.width
                        label: "Caps Lock"
                        value: settings.lockKeysCapsEnabled
                        onToggled: settings.lockKeysCapsEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Num Lock"
                        value: settings.lockKeysNumEnabled
                        onToggled: settings.lockKeysNumEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Scroll Lock"
                        value: settings.lockKeysScrollEnabled
                        onToggled: settings.lockKeysScrollEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Insert"
                        value: settings.lockKeysInsertEnabled
                        onToggled: settings.lockKeysInsertEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Bold text"
                        description: "Use a bold font for the key name."
                        value: settings.lockKeysBoldUi
                        onToggled: settings.lockKeysBoldUi = value
                    }
                    SettingSlider {
                        width: parent.width
                        label: "Duration"
                        description: "How long the lock keys flyout stays on screen."
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.lockKeysDuration
                        valueText: (settings.lockKeysDuration / 1000).toFixed(1) + " s"
                        onMoved: settings.lockKeysDuration = Math.round(value)
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
                    SettingCombo {
                        width: parent.width
                        label: "Widget side"
                        description: "Which part of the taskbar the widget docks to."
                        model: ["Left", "Center", "Right"]
                        currentIndex: settings.taskbarWidgetSide
                        onActivated: settings.taskbarWidgetSide = index
                    }
                    SettingRow {
                        width: parent.width
                        label: "Pause overlay"
                        description: "Dim the album art with a pause glyph when playback is paused."
                        value: settings.taskbarWidgetShowPauseOverlay
                        onToggled: settings.taskbarWidgetShowPauseOverlay = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Scroll long titles"
                        description: "Marquee-scroll the track title when it is too long to fit."
                        value: settings.taskbarWidgetScrollingEnabled
                        onToggled: settings.taskbarWidgetScrollingEnabled = value
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Widget monitor"
                        description: "Which display's taskbar the widget docks to."
                        model: placement.monitorNames()
                        currentIndex: settings.taskbarWidgetMonitor
                        onActivated: settings.taskbarWidgetMonitor = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: "Widget offset"
                        // centered widgets can be nudged either way; edge-docked only inward
                        description: settings.taskbarWidgetSide === 1
                            ? "Shift the centered widget left or right along the taskbar."
                            : "Fine-tune the widget's horizontal position on the taskbar."
                        from: settings.taskbarWidgetSide === 1 ? -600 : 0
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
                    SettingRow {
                        width: parent.width
                        label: "Tray left-click shows media flyout"
                        description: "Left-clicking the tray icon shows the media flyout instead of the dashboard."
                        value: settings.trayLeftClickShowsFlyout
                        onToggled: settings.trayLeftClickShowsFlyout = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Hide tray icon"
                        description: "Hide the notification-area icon (re-launch the app to reopen the dashboard)."
                        value: settings.trayIconHidden
                        onToggled: settings.trayIconHidden = value
                    }

                    Row {
                        spacing: 12
                        topPadding: 8
                        Button {
                            text: "Export settings…"
                            onClicked: exportDialog.open()
                        }
                        Button {
                            text: "Import settings…"
                            onClicked: importDialog.open()
                        }
                    }
                }
                }
            }
        }
    }

    FileDialog {
        id: exportDialog
        title: "Export settings"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Settings files (*.ini)"]
        defaultSuffix: "ini"
        onAccepted: settings.exportTo(selectedFile)
    }
    FileDialog {
        id: importDialog
        title: "Import settings"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Settings files (*.ini)"]
        onAccepted: settings.importFrom(selectedFile)
    }
}
