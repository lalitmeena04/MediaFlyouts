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
    color: theme.window

    // keep the title bar matching the theme
    function applyChrome() {
        windowEffects.setDarkFrame(dash, theme.dark)
    }
    Component.onCompleted: applyChrome()
    onVisibleChanged: if(visible) applyChrome()
    property Connections _chromeThemeConn: Connections {
        target: theme
        function onChanged() { dash.applyChrome() }
    }

    property int currentIndex: 0
    readonly property var peripheralSections: [
        { "label": "2.4 GHz", "devices": batteryStatus.devices.filter(function(device) {
            return device.connectionKind === "wireless"
        }) },
        { "label": "Bluetooth", "devices": batteryStatus.devices.filter(function(device) {
            return device.connectionKind === "bluetooth" || device.connectionKind === "ble"
        }) }
    ].filter(function(section) { return section.devices.length > 0 })
    readonly property var pages: [
        { "icon": Fluent.musicNote, "name": "Home",         "desc": "Overview of MediaFlyouts and what's currently playing." },
        { "icon": Fluent.play,      "name": "Media Flyout", "desc": "Configure the now-playing flyout that appears on media keys." },
        { "icon": Fluent.battery,   "name": "Peripheral Flyout", "desc": "Announce peripherals as they connect, disconnect or run low on battery." },
        { "icon": Fluent.musicNote, "name": "Media Widget", "desc": "A compact now-playing widget docked on the taskbar." },
        { "icon": Fluent.battery,   "name": "Peripheral Widget", "desc": "Battery rings for your wireless peripherals, docked on the taskbar." },
        { "icon": Fluent.speaker2,  "name": "Volume",       "desc": "Configure the volume flyout and native OSD override." },
        { "icon": Fluent.keyboard,  "name": "Lock Keys",    "desc": "Caps / Num / Scroll lock indicator flyouts." },
        { "icon": Fluent.dismiss,   "name": "App Filtering", "desc": "Choose which media apps are allowed to show flyouts." },
        { "icon": Fluent.settings,  "name": "Additional Settings", "desc": "Theme, accent, startup, tray and settings backup." },
        { "icon": Fluent.info,      "name": "About",        "desc": "App info and updates." }
    ]

    // searchable index of settings -> the page that hosts them
    readonly property var searchIndex: [
        { "label": "Seekbar", "page": 1 }, { "label": "Interactive seekbar", "page": 1 },
        { "label": "Compact layout", "page": 1 }, { "label": "Center title and artist", "page": 1 },
        { "label": "Always display", "page": 1 }, { "label": "Show player name", "page": 1 },
        { "label": "Accent from album art", "page": 1 }, { "label": "Repeat button", "page": 1 },
        { "label": "Shuffle button", "page": 1 }, { "label": "Up next flyout", "page": 1 },
        { "label": "Hide during fullscreen", "page": 1 }, { "label": "Flyout position", "page": 1 },
        { "label": "Monitor", "page": 1 }, { "label": "Flyout duration", "page": 1 },
        { "label": "Animation speed", "page": 1 }, { "label": "Animation easing", "page": 1 },
        { "label": "Volume flyout", "page": 5 }, { "label": "Override native OSD", "page": 5 },
        { "label": "Volume flyout duration", "page": 5 }, { "label": "Lock keys flyout", "page": 6 },
        { "label": "Caps lock", "page": 6 }, { "label": "Num lock", "page": 6 },
        { "label": "Scroll lock", "page": 6 }, { "label": "Insert key", "page": 6 },
        { "label": "Bold indicator text", "page": 6 }, { "label": "Lock keys duration", "page": 6 },
        { "label": "Media widget", "page": 3 }, { "label": "Widget controls", "page": 3 },
        { "label": "Widget alignment", "page": 3 }, { "label": "Widget offset", "page": 3 },
        { "label": "Pause overlay", "page": 3 }, { "label": "Scroll long titles", "page": 3 },
        { "label": "Audio visualizer", "page": 3 },
        { "label": "Peripheral flyout", "page": 2 }, { "label": "Low battery warning", "page": 2 },
        { "label": "Peripherals widget", "page": 4 }, { "label": "Battery rings", "page": 4 },
        { "label": "Blacklist", "page": 7 },
        { "label": "Whitelist", "page": 7 }, { "label": "Filter mode", "page": 7 },
        { "label": "Theme", "page": 8 }, { "label": "Accent color", "page": 8 },
        { "label": "Launch on startup", "page": 8 }, { "label": "Hide tray icon", "page": 8 },
        { "label": "Export settings", "page": 8 }, { "label": "Import settings", "page": 8 },
        { "label": "Check for updates", "page": 9 }, { "label": "Version", "page": 9 }
    ]

    readonly property var searchResults: {
        var q = searchQuery.toLowerCase()
        if(q.length === 0)
            return []
        return searchIndex.filter(function(e) { return e.label.toLowerCase().indexOf(q) >= 0 })
    }
    property string searchQuery: ""

    // hide to tray instead of quitting the app
    onClosing: (event) => {
        event.accepted = false
        dash.hide()
    }

    Row {
        anchors.fill: parent

        Rectangle {
            id: sidebar
            property bool collapsed: false
            width: collapsed ? 64 : 240
            height: parent.height
            color: theme.sidebar

            Behavior on width {
                NumberAnimation { duration: 140; easing.type: Easing.OutQuad }
            }

            Item {
                anchors.fill: parent
                anchors.margins: sidebar.collapsed ? 8 : 18

                Row {
                    id: sidebarHeader
                    anchors.top: parent.top
                    anchors.left: parent.left
                    spacing: 12
                    bottomPadding: 12
                    IconLabel {
                        glyph: Fluent.musicNote
                        size: 24
                        color: theme.accent
                    }
                    Text {
                        visible: !sidebar.collapsed
                        text: "MediaFlyouts"
                        color: theme.textPrimary
                        font.pixelSize: 18
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                TextField {
                    id: searchField
                    visible: !sidebar.collapsed
                    anchors.top: sidebarHeader.bottom
                    anchors.topMargin: 8
                    width: parent.width
                    placeholderText: "Search settings…"
                    color: theme.textPrimary
                    placeholderTextColor: theme.textMuted
                    background: Rectangle {
                        radius: 6
                        color: theme.card
                        border.color: theme.border
                    }
                    onTextChanged: dash.searchQuery = text
                }

                ScrollView {
                    id: navigationScroll
                    anchors.top: sidebar.collapsed ? sidebarHeader.bottom : searchField.bottom
                    anchors.topMargin: 8
                    anchors.bottom: collapseButton.top
                    anchors.bottomMargin: 8
                    anchors.left: parent.left
                    anchors.right: parent.right
                    clip: true

                    // search results replace the nav list while searching
                    Column {
                    width: navigationScroll.availableWidth
                    spacing: 4
                    Column {
                        width: parent.width
                        visible: dash.searchQuery.length > 0
                        spacing: 4
                        Repeater {
                            model: dash.searchResults
                            delegate: Rectangle {
                            required property var modelData
                            width: parent.width
                            height: 38
                            radius: 8
                            color: resMouse.containsMouse ? theme.cardHover : "transparent"
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 12
                                Text {
                                    text: modelData.label
                                    color: theme.textPrimary
                                    font.pixelSize: 13
                                }
                                Text {
                                    text: dash.pages[modelData.page].name
                                    color: theme.textMuted
                                    font.pixelSize: 10
                                }
                            }
                            MouseArea {
                                id: resMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    dash.currentIndex = modelData.page
                                    searchField.text = ""
                                }
                            }
                            }
                        }
                        Text {
                            visible: dash.searchResults.length === 0
                            text: "No matching settings."
                            color: theme.textMuted
                            font.pixelSize: 12
                            leftPadding: 12
                        }
                    }

                    Column {
                        width: parent.width
                        visible: dash.searchQuery.length === 0
                        spacing: 4
                        Repeater {
                            model: dash.pages

                            delegate: Rectangle {
                        required property int index
                        required property var modelData

                        width: sidebar.collapsed ? 40 : sidebar.width - 36
                        visible: dash.searchQuery.length === 0
                        height: visible ? 40 : 0
                        radius: 8
                        color: dash.currentIndex === index ? theme.cardHover : (navMouse.containsMouse ? theme.card : "transparent")

                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: sidebar.collapsed ? 11 : 12
                            spacing: 12
                            IconLabel {
                                glyph: modelData.icon
                                size: 18
                                color: dash.currentIndex === index ? theme.accent : theme.textSecondary
                            }
                            Text {
                                visible: !sidebar.collapsed
                                text: modelData.name
                                color: theme.textPrimary
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
                }

                Rectangle {
                    id: collapseButton
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    width: sidebar.collapsed ? 40 : parent.width
                    height: 32
                    radius: 8
                    color: collapseMouse.containsMouse ? theme.card : "transparent"

                    IconLabel {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: sidebar.collapsed ? 11 : 12
                        glyph: sidebar.collapsed ? Fluent.chevronDoubleRight : Fluent.chevronDoubleLeft
                        size: 16
                        color: theme.textSecondary
                    }
                    Text {
                        visible: !sidebar.collapsed
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 40
                        text: "Collapse"
                        color: theme.textMuted
                        font.pixelSize: 12
                    }
                    MouseArea {
                        id: collapseMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            sidebar.collapsed = !sidebar.collapsed
                            if(sidebar.collapsed)
                                searchField.text = ""
                        }
                    }
                }
            }
        }

        Rectangle {
            width: dash.width - sidebar.width
            height: parent.height
            color: theme.window

            Text {
                id: pageTitle
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 32
                text: dash.pages[dash.currentIndex].name
                color: theme.textPrimary
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
                color: theme.textSecondary
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
                            color: theme.card
                            border.color: theme.border

                            Row {
                                anchors.fill: parent
                                anchors.margins: 18
                                spacing: 18

                                Rectangle {
                                    width: 108
                                    height: 108
                                    radius: 10
                                    color: theme.artPlaceholder
                                    clip: true
                                    anchors.verticalCenter: parent.verticalCenter
                                    RoundedArt {
                                        anchors.fill: parent
                                        source: nowPlaying.artSource
                                        visible: nowPlaying.hasArt
                                        radius: 10
                                    }
                                    IconLabel {
                                        anchors.centerIn: parent
                                        visible: !nowPlaying.hasArt
                                        glyph: Fluent.musicNote
                                        size: 44
                                        color: theme.textTertiary
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
                                        color: theme.textPrimary
                                        font.pixelSize: 20
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        width: parent.width
                                        text: nowPlaying.hasMedia ? nowPlaying.artist : "Start media in any app to control it here"
                                        color: theme.textSecondary
                                        font.pixelSize: 14
                                        elide: Text.ElideRight
                                    }

                                    Rectangle {
                                        visible: nowPlaying.hasMedia && nowPlaying.playerName.length > 0
                                        radius: 4
                                        color: theme.accentBadge
                                        height: playerText.implicitHeight + 6
                                        width: playerText.implicitWidth + 14
                                        Text {
                                            id: playerText
                                            anchors.centerIn: parent
                                            text: nowPlaying.playerName
                                            color: theme.accentSoft
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
                                            color: theme.track
                                            Rectangle {
                                                width: parent.width * Math.max(0, Math.min(1, nowPlaying.progress))
                                                height: parent.height
                                                radius: 2
                                                color: theme.accent
                                            }
                                        }
                                        Item {
                                            width: parent.width
                                            height: elapsedText.implicitHeight
                                            Text {
                                                id: elapsedText
                                                anchors.left: parent.left
                                                text: nowPlaying.elapsed
                                                color: theme.textTertiary
                                                font.pixelSize: 11
                                            }
                                            Text {
                                                anchors.right: parent.right
                                                text: nowPlaying.duration
                                                color: theme.textTertiary
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
                                            color: prevMa.containsMouse ? theme.textPrimary : theme.textSecondary
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
                                            color: playMa.containsMouse ? theme.textPrimary : theme.accent
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
                                            color: nextMa.containsMouse ? theme.textPrimary : theme.textSecondary
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
                            color: theme.textPrimary
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
                                    { "name": "Volume flyout", "icon": Fluent.speaker2, "key": "volume", "page": 5 },
                                    { "name": "Lock keys", "icon": Fluent.keyboard, "key": "lock", "page": 6 },
                                    { "name": "Media widget", "icon": Fluent.musicNote, "key": "widget", "page": 3 },
                                    { "name": "Up next", "icon": Fluent.next, "key": "nextup", "page": 1 },
                                    { "name": "Native OSD override", "icon": Fluent.speaker2, "key": "osd", "page": 5 }
                                ]
                                delegate: Rectangle {
                                    required property var modelData
                                    width: (homePage.width - 24) / 3
                                    height: 76
                                    radius: 10
                                    color: cardMa.containsMouse ? theme.cardHover : theme.card
                                    border.color: theme.border

                                    Row {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 10
                                        IconLabel {
                                            anchors.verticalCenter: parent.verticalCenter
                                            glyph: modelData.icon
                                            size: 22
                                            color: homePage.featureOn(modelData.key) ? theme.accent : theme.textMuted
                                        }
                                        Column {
                                            anchors.verticalCenter: parent.verticalCenter
                                            spacing: 4
                                            width: parent.width - 32
                                            Text {
                                                width: parent.width
                                                text: modelData.name
                                                color: theme.textPrimary
                                                font.pixelSize: 13
                                                font.bold: true
                                                elide: Text.ElideRight
                                            }
                                            Text {
                                                text: homePage.featureOn(modelData.key) ? "On" : "Off"
                                                color: homePage.featureOn(modelData.key) ? "#6FCF7F" : theme.textMuted
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

                        // wireless peripherals with a readable battery level (Bluetooth today; see BatteryStatusController)
                        Text {
                            visible: batteryStatus.devices.length > 0
                            text: "Peripherals"
                            color: theme.textPrimary
                            font.pixelSize: 16
                            font.bold: true
                            topPadding: 4
                        }

                        Repeater {
                            width: parent.width
                            model: dash.peripheralSections
                            delegate: Column {
                                required property var modelData
                                width: parent.width
                                spacing: 8

                                Text {
                                    text: modelData.label
                                    color: theme.textSecondary
                                    font.pixelSize: 13
                                    font.bold: true
                                }

                                Flow {
                                    width: parent.width
                                    spacing: 12

                                    Repeater {
                                        model: modelData.devices
                                        delegate: Rectangle {
                                    required property var modelData
                                    width: 108
                                    height: modelData.components.length > 1 ? 168 : 128
                                    radius: 12
                                    color: theme.card
                                    border.color: theme.border

                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 6

                                        BatteryRing {
                                            id: ring
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            width: 64
                                            height: 64
                                            thickness: 5
                                            percent: modelData.percent

                                            Text {
                                                anchors.centerIn: parent
                                                text: DeviceIcons.glyph(modelData.name, modelData.connectionKind, modelData.category)
                                                font.family: DeviceIcons.family(modelData.name, modelData.connectionKind, modelData.category)
                                                font.pixelSize: 26
                                                color: theme.accent
                                            }
                                        }

                                        Row {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            spacing: 4
                                            Text {
                                                text: Fluent.battery
                                                font.family: Fluent.fontFamily
                                                font.pixelSize: 12
                                                color: modelData.percent >= 0 ? ring.progressColor : theme.textMuted
                                            }
                                            Text {
                                                text: modelData.percent >= 0
                                                          ? modelData.percent + "%" + (modelData.charging ? " \u26A1" : "")
                                                          : "Unknown"
                                                color: theme.textPrimary
                                                font.pixelSize: 12
                                                font.bold: true
                                            }
                                        }

                                        Row {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            spacing: 8
                                            visible: modelData.components.length > 1

                                            Repeater {
                                                model: modelData.components
                                                delegate: Column {
                                                    required property var modelData
                                                    spacing: 1

                                                    BatteryRing {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        width: 22
                                                        height: 22
                                                        thickness: 2
                                                        percent: modelData.percent
                                                    }
                                                    Text {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.label + (modelData.charging ? " \u26A1" : "")
                                                        color: theme.textMuted
                                                        font.pixelSize: 9
                                                    }
                                                    Text {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.percent >= 0 ? modelData.percent + "%" : "Unknown"
                                                        color: theme.textPrimary
                                                        font.pixelSize: 9
                                                    }
                                                }
                                            }
                                        }

                                        Text {
                                            width: 100
                                            horizontalAlignment: Text.AlignHCenter
                                            text: modelData.name
                                            color: theme.textMuted
                                            font.pixelSize: 10
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                                }
                            }
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: "Tip: press a media or volume key, or click the tray icon, to preview the flyouts."
                            color: theme.textMuted
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
                        label: "Accent from album art"
                        description: "Tint the flyout accents using the current album art's dominant color."
                        value: settings.accentFromArt
                        onToggled: settings.accentFromArt = value
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
                        description: "Suppress flyouts while a fullscreen video game is in the foreground."
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
                    SettingCombo {
                        width: parent.width
                        label: "Up next style"
                        description: "How the up-next flyout is laid out."
                        model: ["Now playing card", "Compact pill"]
                        currentIndex: settings.nextUpStyle
                        onActivated: settings.nextUpStyle = index
                    }
                }

                // Volume page
                Column {
                    visible: dash.currentIndex === 5
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
                    SettingRow {
                        width: parent.width
                        label: "App mixer in volume flyout"
                        description: "Show a button on the volume flyout to expand a per-app volume mixer."
                        value: settings.volumeFlyoutMixerEnabled
                        onToggled: settings.volumeFlyoutMixerEnabled = value
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Volume flyout position"
                        description: "Place the volume flyout independently of the media flyout."
                        model: ["Top left", "Top center", "Top right", "Bottom left", "Bottom center", "Bottom right", "Above taskbar widget"]
                        currentIndex: settings.volumeFlyoutPosition
                        onActivated: settings.volumeFlyoutPosition = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Volume flyout monitor"
                        description: "Which display the volume flyout is shown on."
                        model: placement.monitorNames()
                        currentIndex: settings.volumeFlyoutMonitor
                        onActivated: settings.volumeFlyoutMonitor = index
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
                    visible: dash.currentIndex === 6
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

                // Media Widget page
                Column {
                    visible: dash.currentIndex === 3
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
                    SettingRow {
                        width: parent.width
                        label: "Audio visualizer"
                        description: "Show a live spectrum of the system audio behind the widget."
                        value: settings.taskbarVisualizerEnabled
                        onToggled: settings.taskbarVisualizerEnabled = value
                    }
                    SettingCombo {
                        visible: settings.taskbarVisualizerEnabled
                        x: 16
                        width: parent.width - 16
                        label: "Visualizer style"
                        description: "How the audio is drawn behind the widget."
                        model: ["Bars", "Radial", "Waveform", "Particles"]
                        currentIndex: settings.visualizerStyle
                        onActivated: settings.visualizerStyle = index
                    }
                    SettingRow {
                        visible: settings.taskbarVisualizerEnabled && settings.visualizerStyle === 0
                        x: 16
                        width: parent.width - 16
                        label: "Mirror the equalizer"
                        description: "Grow the bars symmetrically from the middle instead of the bottom."
                        value: settings.taskbarVisualizerCentered
                        onToggled: settings.taskbarVisualizerCentered = value
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

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "Battery rings for your peripherals are configured on the Peripheral Widget page."
                        color: theme.textMuted
                        font.pixelSize: 12
                        topPadding: 8
                    }
                }

                // Peripheral Widget page
                Column {
                    visible: dash.currentIndex === 4
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: "Peripherals widget"
                        description: "Dock a battery widget for selected wireless peripherals on the taskbar."
                        value: settings.peripheralsWidgetEnabled
                        onToggled: settings.peripheralsWidgetEnabled = value
                    }
                    SettingRow {
                        visible: settings.peripheralsWidgetEnabled
                        width: parent.width
                        label: "Match app accent"
                        description: "Color the battery rings using the app accent (or album art, if that's enabled) instead of green/yellow/red by charge level."
                        value: settings.batteryRingUseAccent
                        onToggled: settings.batteryRingUseAccent = value
                    }

                    Text {
                        visible: settings.peripheralsWidgetEnabled
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "Choose which detected peripherals to show on the widget:"
                        color: theme.textSecondary
                        font.pixelSize: 12
                        topPadding: 4
                    }
                    Text {
                        visible: settings.peripheralsWidgetEnabled && batteryStatus.devices.length === 0
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "No peripherals are currently connected."
                        color: theme.textMuted
                        font.pixelSize: 12
                    }
                    Repeater {
                        model: settings.peripheralsWidgetEnabled ? batteryStatus.devices : []
                        delegate: SettingRow {
                            required property var modelData
                            width: parent.width
                            label: modelData.name
                            description: {
                                var text = modelData.percent >= 0 ? "Currently at " + modelData.percent + "%" : "Battery level unknown"
                                var parts = []
                                for(var i = 0; i < modelData.components.length; i++)
                                    parts.push(modelData.components[i].label + " " + modelData.components[i].percent + "%")
                                if(parts.length > 1)
                                    text += " (" + parts.join(", ") + ")"
                                var link = DeviceIcons.transport(modelData.connectionKind)
                                return link ? text + ", over " + link + "." : text + "."
                            }
                            value: settings.peripheralSelected(modelData.name)
                            onToggled: settings.setPeripheralSelected(modelData.name, value)
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "Connect and low-battery announcements are configured on the Peripheral Flyout page."
                        color: theme.textMuted
                        font.pixelSize: 12
                        topPadding: 8
                    }
                }

                // Peripheral Flyout page
                Column {
                    visible: dash.currentIndex === 2
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: "Peripheral flyout"
                        description: "Show a flyout when a peripheral connects, disconnects or runs low on battery."
                        value: settings.peripheralFlyoutEnabled
                        onToggled: settings.peripheralFlyoutEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "On connect"
                        description: "Announce a peripheral when it connects."
                        value: settings.peripheralFlyoutOnConnect
                        onToggled: settings.peripheralFlyoutOnConnect = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "On disconnect"
                        description: "Announce a peripheral when it disconnects."
                        value: settings.peripheralFlyoutOnDisconnect
                        onToggled: settings.peripheralFlyoutOnDisconnect = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "On low battery"
                        description: "Warn once when a battery drops below 20%, and again below 10%."
                        value: settings.peripheralFlyoutOnLowBattery
                        onToggled: settings.peripheralFlyoutOnLowBattery = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "On charging"
                        description: "Announce when a peripheral starts or stops charging."
                        value: settings.peripheralFlyoutOnCharging
                        onToggled: settings.peripheralFlyoutOnCharging = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Low battery notification"
                        description: "Also show a tray notification for a low battery, so it isn't missed."
                        value: settings.peripheralLowBatteryToast
                        onToggled: settings.peripheralLowBatteryToast = value
                    }
                    SettingRow {
                        width: parent.width
                        label: "Hide during fullscreen"
                        description: "Suppress flyouts while a fullscreen video game is in the foreground. Shared with the other flyouts."
                        value: settings.disableOnFullscreen
                        onToggled: settings.disableOnFullscreen = value
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Peripheral flyout position"
                        description: "Place the peripheral flyout independently of the other flyouts."
                        model: ["Top left", "Top center", "Top right", "Bottom left", "Bottom center", "Bottom right", "Above taskbar widget"]
                        currentIndex: settings.peripheralFlyoutPosition
                        onActivated: settings.peripheralFlyoutPosition = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Peripheral flyout monitor"
                        description: "Which display the peripheral flyout is shown on."
                        model: placement.monitorNames()
                        currentIndex: settings.peripheralFlyoutMonitor
                        onActivated: settings.peripheralFlyoutMonitor = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: "Peripheral flyout duration"
                        description: "How long the peripheral flyout stays on screen."
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.peripheralFlyoutDuration
                        valueText: (settings.peripheralFlyoutDuration / 1000).toFixed(1) + " s"
                        onMoved: settings.peripheralFlyoutDuration = Math.round(value)
                    }
                }

                // App Filtering page
                Column {
                    visible: dash.currentIndex === 7
                    width: parent.width
                    spacing: 12

                    SettingCombo {
                        width: parent.width
                        label: "Filter mode"
                        description: "Off shows every app. Blacklist hides the listed apps; whitelist shows only the listed apps."
                        model: ["Off", "Blacklist (hide these)", "Whitelist (show only these)"]
                        currentIndex: settings.filterMode
                        onActivated: settings.filterMode = index
                    }

                    Row {
                        visible: settings.filterMode !== 0
                        width: parent.width
                        spacing: 8

                        TextField {
                            id: filterInput
                            width: parent.width - addBtn.width - 8
                            placeholderText: nowPlaying.playerName.length > 0
                                ? "App name (e.g. " + nowPlaying.playerName + ")"
                                : "App name (e.g. Spotify, chrome)"
                            color: theme.textPrimary
                            placeholderTextColor: theme.textMuted
                            background: Rectangle {
                                radius: 6
                                color: theme.card
                                border.color: theme.border
                            }
                            onAccepted: { settings.addFilterEntry(text); text = "" }
                        }
                        Button {
                            id: addBtn
                            text: "Add"
                            onClicked: { settings.addFilterEntry(filterInput.text); filterInput.text = "" }
                        }
                    }

                    Text {
                        visible: settings.filterMode !== 0 && nowPlaying.playerName.length > 0
                        width: parent.width
                        text: "Now playing app: " + nowPlaying.playerName
                        color: theme.textMuted
                        font.pixelSize: 12
                    }

                    Column {
                        visible: settings.filterMode !== 0
                        width: parent.width
                        spacing: 6

                        Text {
                            visible: settings.filterList.length === 0
                            text: "No apps added yet."
                            color: theme.textMuted
                            font.pixelSize: 12
                        }

                        Repeater {
                            model: settings.filterList
                            delegate: Rectangle {
                                required property string modelData
                                width: parent.width
                                height: 40
                                radius: 8
                                color: theme.card
                                border.color: theme.border

                                Text {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData
                                    color: theme.textPrimary
                                    font.pixelSize: 13
                                }
                                IconLabel {
                                    anchors.right: parent.right
                                    anchors.rightMargin: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                    glyph: Fluent.dismiss
                                    size: 14
                                    color: rmArea.containsMouse ? theme.textPrimary : theme.textTertiary
                                    MouseArea {
                                        id: rmArea
                                        anchors.fill: parent
                                        anchors.margins: -8
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: settings.removeFilterEntry(modelData)
                                    }
                                }
                            }
                        }
                    }
                }

                // Additional Settings page
                Column {
                    visible: dash.currentIndex === 8
                    width: parent.width
                    spacing: 12

                    SettingCombo {
                        width: parent.width
                        label: "Theme"
                        description: "Match the system theme, or force light or dark."
                        model: ["Use system setting", "Light", "Dark"]
                        currentIndex: settings.appTheme
                        onActivated: settings.appTheme = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: "Accent color"
                        description: "Use the built-in accent, match the Windows accent, or pick a custom color."
                        model: ["Default", "Match system", "Custom"]
                        currentIndex: settings.accentMode
                        onActivated: settings.accentMode = index
                    }
                    Row {
                        visible: settings.accentMode === 2
                        x: 16
                        width: parent.width - 16
                        spacing: 12
                        Rectangle {
                            width: 28
                            height: 28
                            radius: 6
                            color: theme.accent
                            border.color: theme.border
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Button {
                            text: "Choose color…"
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: {
                                accentDialog.selectedColor = theme.accent
                                accentDialog.open()
                            }
                        }
                    }
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

                // About page
                Column {
                    visible: dash.currentIndex === 9
                    width: parent.width
                    spacing: 12

                    Row {
                        spacing: 12
                        IconLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            glyph: Fluent.musicNote
                            size: 32
                            color: theme.accent
                        }
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: "MediaFlyouts"
                                color: theme.textPrimary
                                font.pixelSize: 20
                                font.bold: true
                            }
                            Text {
                                text: "Version " + updateChecker.currentVersion
                                color: theme.textMuted
                                font.pixelSize: 12
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "Fluent-style media, volume, lock-key and peripheral overlays for Windows, built with Qt 6."
                        color: theme.textSecondary
                        font.pixelSize: 13
                        topPadding: 4
                    }

                    Row {
                        spacing: 12
                        topPadding: 8
                        Button {
                            text: "Check for updates"
                            enabled: !updateChecker.checking
                            onClicked: updateChecker.check()
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: updateChecker.status
                            color: updateChecker.updateAvailable ? theme.accent : theme.textTertiary
                            font.pixelSize: 13
                        }
                    }

                    SettingRow {
                        width: parent.width
                        label: "Check for updates on launch"
                        description: "Automatically look for a newer release shortly after startup."
                        value: settings.autoCheckUpdates
                        onToggled: settings.autoCheckUpdates = value
                    }

                    // icon-only link to the project page
                    Rectangle {
                        width: 40
                        height: 40
                        radius: 8
                        color: repoHover.containsMouse ? theme.cardHover : theme.card
                        border.color: theme.border

                        IconLabel {
                            anchors.centerIn: parent
                            glyph: Fluent.link
                            size: 18
                            color: theme.textPrimary
                        }
                        MouseArea {
                            id: repoHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://github.com/scleaverzer0ne/MediaFlyouts")
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
    ColorDialog {
        id: accentDialog
        title: "Choose accent color"
        onAccepted: settings.customAccent = selectedColor
    }
}
