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
    property int browserIndex: 0
    readonly property var browserIds: ["chrome", "msedge", "firefox", "brave"]
    onCurrentIndexChanged: pageScroll.contentItem.contentY = 0
    // Pages are found by their stable key, not their translated name.
    function showPage(key) {
        for(var i = 0; i < pages.length; i++) {
            if(pages[i].key === key) {
                currentIndex = i
                break
            }
        }
        show()
        raise()
        requestActivate()
    }
    readonly property var wirelessDevices: batteryStatus.devices.filter(function(device) {
        return device.connectionKind !== "usb"
    })
    readonly property var wiredDevices: batteryStatus.devices.filter(function(device) {
        return device.connectionKind === "usb"
    })
    readonly property var shownDevices: {
        var excluded = settings.peripheralsWidgetExcluded
        return batteryStatus.devices.filter(function(device) {
            return excluded.indexOf(device.name) === -1
        })
    }
    readonly property var peripheralSections: [
        { "label": qsTr("2.4 GHz"), "devices": shownDevices.filter(function(device) {
            return device.connectionKind === "wireless"
        }) },
        { "label": qsTr("Bluetooth"), "devices": shownDevices.filter(function(device) {
            return device.connectionKind === "bluetooth" || device.connectionKind === "ble"
        }) },
        { "label": qsTr("Wired"), "devices": shownDevices.filter(function(device) {
            return device.connectionKind === "usb"
        }) }
    ].filter(function(section) { return section.devices.length > 0 })
    readonly property var pages: [
        { "icon": Fluent.home,      "key": "home",        "name": qsTr("Home"),         "desc": qsTr("Overview of MediaFlyouts and what's currently playing.") },
        { "icon": Fluent.play,      "key": "media",       "name": qsTr("Media Flyout"), "desc": qsTr("Configure the now-playing flyout that appears on media keys.") },
        { "icon": Fluent.plugConnected, "key": "peripheralFlyout", "name": qsTr("Peripheral Flyout"), "desc": qsTr("Announce peripherals as they connect, disconnect or run low on battery.") },
        { "icon": Fluent.charge, "key": "charger", "name": qsTr("Charger Flyout"), "desc": qsTr("Announce the charger as it is plugged in or unplugged.") },
        { "icon": Fluent.musicNote, "key": "taskbarWidget", "name": qsTr("Taskbar Widget"), "desc": qsTr("Configure the media and peripheral widgets docked on the taskbar.") },
        { "icon": Fluent.speaker2,  "key": "volume",      "name": qsTr("Volume"),       "desc": qsTr("Configure the volume flyout and native OSD override.") },
        { "icon": Fluent.keyboard,  "key": "lockKeys",    "name": qsTr("Lock Keys"),    "desc": qsTr("Caps / Num / Scroll lock indicator flyouts.") },
        { "icon": Fluent.filter,    "key": "filtering",   "name": qsTr("App Filtering"), "desc": qsTr("Choose which media apps are allowed to show flyouts.") },
        { "icon": Fluent.settings,  "key": "additional",  "name": qsTr("Additional Settings"), "desc": qsTr("Language, theme, accent, startup, tray and settings backup.") },
        { "icon": Fluent.flash,     "key": "support",     "name": qsTr("Support"),      "desc": qsTr("Say thanks with a donation or a tip.") },
        { "icon": Fluent.info,      "key": "about",       "name": qsTr("About"),        "desc": qsTr("App info, updates and error reports.") }
    ]

    readonly property var searchIndex: [
        { "label": qsTr("Seekbar"), "page": 1 }, { "label": qsTr("Interactive seekbar"), "page": 1 },
        { "label": qsTr("Compact layout"), "page": 1 }, { "label": qsTr("Center title and artist"), "page": 1 },
        { "label": qsTr("Always display"), "page": 1 }, { "label": qsTr("Show player name"), "page": 1 },
        { "label": qsTr("Accent from album art"), "page": 1 }, { "label": qsTr("Apply accent app-wide"), "page": 1 }, { "label": qsTr("Repeat button"), "page": 1 },
        { "label": qsTr("Browser artwork"), "page": 4 },
        { "label": qsTr("Shuffle button"), "page": 1 }, { "label": qsTr("Now Playing flyout"), "page": 1 },
        { "label": qsTr("Hide during fullscreen"), "page": 1 }, { "label": qsTr("Flyout position"), "page": 1 },
        { "label": qsTr("Monitor"), "page": 1 }, { "label": qsTr("Flyout duration"), "page": 1 },
        { "label": qsTr("Animation speed"), "page": 1 }, { "label": qsTr("Animation easing"), "page": 1 },
        { "label": qsTr("Volume flyout"), "page": 5 }, { "label": qsTr("Override native OSD"), "page": 5 },
        { "label": qsTr("Volume flyout duration"), "page": 5 }, { "label": qsTr("Lock keys flyout"), "page": 6 },
        { "label": qsTr("Caps lock"), "page": 6 }, { "label": qsTr("Num lock"), "page": 6 },
        { "label": qsTr("Scroll lock"), "page": 6 }, { "label": qsTr("Insert key"), "page": 6 },
        { "label": qsTr("Bold indicator text"), "page": 6 }, { "label": qsTr("Lock keys duration"), "page": 6 },
        { "label": qsTr("Media widget"), "page": 4 }, { "label": qsTr("Widget controls"), "page": 4 },
        { "label": qsTr("Widget alignment"), "page": 4 }, { "label": qsTr("Widget offset"), "page": 4 },
        { "label": qsTr("Widget layout"), "page": 4 }, { "label": qsTr("Stack"), "page": 4 },
        { "label": qsTr("Pause overlay"), "page": 4 }, { "label": qsTr("Scroll long titles"), "page": 4 },
        { "label": qsTr("Audio visualizer"), "page": 4 },
        { "label": qsTr("Peripheral flyout"), "page": 2 }, { "label": qsTr("Low battery warning"), "page": 2 },
        { "label": qsTr("Wired devices"), "page": 2 },
        { "label": qsTr("Charger flyout"), "page": 3 }, { "label": qsTr("On plug in"), "page": 3 },
        { "label": qsTr("On unplug"), "page": 3 }, { "label": qsTr("Charger flyout duration"), "page": 3 },
        { "label": qsTr("Peripherals widget"), "page": 4 }, { "label": qsTr("Battery rings"), "page": 4 },
        { "label": qsTr("Wired devices in widget"), "page": 4 },
        { "label": qsTr("Blacklist"), "page": 7 },
        { "label": qsTr("Whitelist"), "page": 7 }, { "label": qsTr("Filter mode"), "page": 7 },
        { "label": qsTr("Language"), "page": 8 },
        { "label": qsTr("Theme"), "page": 8 }, { "label": qsTr("Accent color"), "page": 8 }, { "label": qsTr("Pro icon"), "page": 8 },
        { "label": qsTr("Launch on startup"), "page": 8 }, { "label": qsTr("Hide tray icon"), "page": 8 },
        { "label": qsTr("Export settings"), "page": 8 }, { "label": qsTr("Import settings"), "page": 8 },
        { "label": qsTr("Restore default settings"), "page": 8 },
        { "label": qsTr("Donate"), "page": 9 }, { "label": qsTr("Tip"), "page": 9 },
        { "label": qsTr("Check for updates"), "page": 10 }, { "label": qsTr("Version"), "page": 10 },
        { "label": qsTr("What's new"), "page": 10 },
        { "label": qsTr("Report a problem"), "page": 10 }, { "label": qsTr("Error report"), "page": 10 }
    ]

    readonly property var searchResults: {
        var q = searchQuery.toLowerCase()
        if(q.length === 0)
            return []
        return searchIndex.filter(function(e) { return e.label.toLowerCase().indexOf(q) >= 0 })
    }
    property string searchQuery: ""
    property bool sidebarSearchOpen: false

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
                id: sidebarContent
                anchors.fill: parent
                anchors.leftMargin: sidebar.collapsed ? 8 : 18
                anchors.topMargin: 18
                anchors.rightMargin: sidebar.collapsed ? 8 : 18
                anchors.bottomMargin: 48

                Row {
                    id: sidebarHeader
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.leftMargin: sidebar.collapsed ? 8 : 9
                    spacing: 12
                    bottomPadding: 12
                    AppLogo {
                        size: 24
                        anchors.verticalCenter: parent.verticalCenter
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
                    placeholderText: qsTr("Search settings…")
                    color: theme.textPrimary
                    placeholderTextColor: theme.textMuted
                    background: Rectangle {
                        radius: 6
                        color: theme.card
                        border.color: theme.border
                    }
                    onTextChanged: dash.searchQuery = text
                }

                Rectangle {
                    id: sidebarSearchButton
                    visible: sidebar.collapsed
                    anchors.top: sidebarHeader.bottom
                    anchors.topMargin: 8
                    width: 40
                    height: 32
                    radius: 8
                    color: sidebarSearchMouse.containsMouse ? theme.card : "transparent"

                    IconLabel {
                        anchors.centerIn: parent
                        glyph: Fluent.search
                        size: 16
                        color: theme.textSecondary
                    }
                    MouseArea {
                        id: sidebarSearchMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            dash.sidebarSearchOpen = !dash.sidebarSearchOpen
                            if(dash.sidebarSearchOpen) {
                                popupSearchField.text = ""
                                sidebarSearchPopup.open()
                            } else {
                                sidebarSearchPopup.close()
                            }
                        }
                        InfoTooltip {
                            text: qsTr("Search settings")
                            hovered: sidebar.collapsed && sidebarSearchMouse.containsMouse
                        }
                    }                }

                ScrollView {
                    id: navigationScroll
                    anchors.top: sidebar.collapsed ? sidebarSearchButton.bottom : searchField.bottom
                    anchors.topMargin: 8
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    clip: true

                    Column {
                    width: navigationScroll.availableWidth
                    spacing: 4
                    Column {
                        width: parent.width
                        visible: dash.searchQuery.length > 0 && !sidebar.collapsed
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
                            text: qsTr("No matching settings.")
                            color: theme.textMuted
                            font.pixelSize: 12
                            leftPadding: 12
                        }
                    }

                    Column {
                        width: parent.width
                        visible: dash.searchQuery.length === 0 || sidebar.collapsed
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
                                width: 18
                                height: 18
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
                            InfoTooltip {
                                text: modelData.name
                                hovered: sidebar.collapsed && navMouse.containsMouse
                            }
                        }
                            }
                        }
                    }
                    }
                }

            }

            Rectangle {
                id: collapseButton
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8
                anchors.left: parent.left
                anchors.leftMargin: 8
                width: sidebar.collapsed ? 40 : sidebar.width - 16
                height: 32
                radius: 8
                color: collapseMouse.containsMouse ? theme.card : "transparent"

                IconLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 11
                    glyph: sidebar.collapsed ? Fluent.chevronDoubleRight : Fluent.chevronDoubleLeft
                    size: 16
                    color: theme.textSecondary
                }
                Text {
                    visible: !sidebar.collapsed
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 40
                    color: theme.textMuted
                    font.pixelSize: 12
                }
                MouseArea {
                    id: collapseMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    InfoTooltip {
                        text: qsTr("Expand")
                        hovered: sidebar.collapsed && collapseMouse.containsMouse
                    }
                    onClicked: {
                        sidebar.collapsed = !sidebar.collapsed
                        if(sidebar.collapsed) {
                            searchField.text = ""
                            dash.searchQuery = ""
                            sidebarSearchPopup.close()
                        }
                    }
                }
            }
        }

    Popup {
        id: sidebarSearchPopup
        parent: sidebarSearchButton
        x: sidebarSearchButton.width + 8
        y: 0
        width: 280
        height: Math.min(360, dash.height - 120)
        padding: 8
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onOpened: popupSearchField.forceActiveFocus()
        onClosed: {
            dash.sidebarSearchOpen = false
            dash.searchQuery = ""
        }
        background: Rectangle {
            radius: 8
            color: theme.card
            border.color: theme.border
        }
        contentItem: Column {
            spacing: 8
            TextField {
                id: popupSearchField
                width: parent.width
                placeholderText: qsTr("Search settings…")
                color: theme.textPrimary
                placeholderTextColor: theme.textMuted
                background: Rectangle {
                    radius: 6
                    color: theme.window
                    border.color: theme.border
                }
                onTextChanged: dash.searchQuery = text
            }
            ScrollView {
                id: popupResultsScroll
                width: parent.width
                height: parent.height - popupSearchField.height - parent.spacing
                clip: true
                Column {
                    width: popupResultsScroll.availableWidth
                    spacing: 4
                    Repeater {
                        model: dash.searchResults
                        delegate: Rectangle {
                            required property var modelData
                            width: parent.width
                            height: 38
                            radius: 6
                            color: popupResultMouse.containsMouse ? theme.cardHover : "transparent"
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
                                id: popupResultMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    dash.currentIndex = modelData.page
                                    sidebarSearchPopup.close()
                                }
                            }
                        }
                    }
                    Text {
                        visible: dash.searchResults.length === 0
                        text: qsTr("No matching settings.")
                        color: theme.textMuted
                        font.pixelSize: 12
                        leftPadding: 12
                    }
                }
            }
        }
    }

        Rectangle {
            width: dash.width - sidebar.width
            height: parent.height
            color: theme.window

            IconLabel {
                id: pageIcon
                anchors.left: parent.left
                anchors.leftMargin: 32
                anchors.verticalCenter: pageTitle.verticalCenter
                glyph: dash.pages[dash.currentIndex].icon
                size: 26
                color: theme.accent
            }

            Text {
                id: pageTitle
                anchors.top: parent.top
                anchors.left: pageIcon.right
                anchors.right: parent.right
                anchors.topMargin: 32
                anchors.leftMargin: 12
                anchors.rightMargin: 32
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

            Rectangle {
                id: statusBanner
                anchors.top: pageDesc.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                anchors.topMargin: visible ? 16 : 0
                visible: capabilities.degraded
                height: visible ? bannerRow.implicitHeight + 24 : 0
                radius: 8
                color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.12)
                border.width: 1
                border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.35)

                Row {
                    id: bannerRow
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    IconLabel {
                        glyph: "\uE9CE"
                        size: 18
                        color: theme.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        width: parent.width - 28
                        text: qsTr("Some features are unavailable because Windows did not provide: %1. Related settings are disabled.")
                              .arg(capabilities.unavailableNames.join(", "))
                        color: theme.textPrimary
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            ScrollView {
                id: pageScroll
                anchors.top: statusBanner.bottom
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

                        function featureOn(key) {
                            switch(key) {
                            case "media": return settings.mediaFlyoutEnabled
                            case "volume": return settings.volumeFlyoutEnabled
                            case "lock": return settings.lockKeysEnabled
                            case "widget": return settings.taskbarWidgetEnabled
                            case "nextup": return settings.nextUpEnabled
                            case "osd": return settings.overrideNativeOsd
                            case "charger": return settings.chargerFlyoutEnabled
                            }
                            return false
                        }

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
                                        text: nowPlaying.hasMedia ? nowPlaying.title : qsTr("Nothing playing")
                                        color: theme.textPrimary
                                        font.pixelSize: 20
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        width: parent.width
                                        text: nowPlaying.hasMedia ? nowPlaying.artist : qsTr("Start media in any app to control it here")
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

                        UpgradeCard {
                            width: parent.width
                            ownerWindow: dash
                        }

                        Text {
                            text: qsTr("At a glance")
                            color: theme.textPrimary
                            font.pixelSize: 16
                            font.bold: true
                            topPadding: 4
                        }

                        Flow {
                            width: parent.width
                            spacing: 12

                            Repeater {
                                model: [
                                    { "name": qsTr("Media flyout"), "icon": Fluent.play, "key": "media", "page": 1 },
                                    { "name": qsTr("Volume flyout"), "icon": Fluent.speaker2, "key": "volume", "page": 5 },
                                    { "name": qsTr("Lock keys"), "icon": Fluent.keyboard, "key": "lock", "page": 6 },
                                    { "name": qsTr("Media widget"), "icon": Fluent.musicNote, "key": "widget", "page": 4 },
                                    { "name": qsTr("Now Playing"), "icon": Fluent.next, "key": "nextup", "page": 1 },
                                    { "name": qsTr("Native OSD override"), "icon": Fluent.speaker2, "key": "osd", "page": 5 },
                                    { "name": qsTr("Charger flyout"), "icon": Fluent.charge, "key": "charger", "page": 3 }
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
                                                text: homePage.featureOn(modelData.key) ? qsTr("On") : qsTr("Off")
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

                        Text {
                            visible: dash.peripheralSections.length > 0
                            text: qsTr("Peripherals")
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
                                    readonly property bool hasBattery: DeviceIcons.hasBattery(modelData.connectionKind, modelData.percent)
                                    width: 108
                                    height: modelData.components.length > 1 ? 168 : (hasBattery ? 128 : 112)
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
                                            charging: modelData.charging
                                            showGauge: hasBattery

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
                                            visible: hasBattery
                                            BatteryIcon {
                                                anchors.verticalCenter: parent.verticalCenter
                                                implicitWidth: 8
                                                implicitHeight: 13
                                                level: modelData.percent >= 0 ? modelData.percent / 100 : 1
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
                                                        charging: modelData.charging
                                                    }
                                                    Text {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.label + (modelData.charging ? " \u26A1" : "")
                                                        color: theme.textMuted
                                                        font.pixelSize: 9
                                                    }
                                                    Text {
                                                        anchors.horizontalCenter: parent.horizontalCenter
                                                        text: modelData.percent >= 0 ? modelData.percent + "%" : qsTr("Unknown")
                                                        color: theme.textPrimary
                                                        font.pixelSize: 9
                                                    }
                                                }
                                            }
                                        }

                                        Text {
                                            width: 100
                                            horizontalAlignment: Text.AlignHCenter
                                            text: settings.peripheralNames[modelData.name] || modelData.name
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
                            text: qsTr("Tip: press a media or volume key, or click the tray icon, to preview the flyouts.")
                            color: theme.textMuted
                            font.pixelSize: 12
                            topPadding: 4
                        }
                    }

                Column {
                    visible: dash.currentIndex === 1
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: qsTr("Media flyout")
                        description: qsTr("Show the now-playing flyout on media key presses.")
                        available: capabilities.mediaSessionAvailable
                        unavailableNote: qsTr("Unavailable: no system media session was detected on this PC.")
                        value: settings.mediaFlyoutEnabled
                        onToggled: settings.mediaFlyoutEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Seekbar")
                        description: qsTr("Show the progress bar and elapsed / total time.")
                        value: settings.seekbarEnabled
                        onToggled: settings.seekbarEnabled = value
                    }
                    SettingRow {
                        visible: settings.seekbarEnabled
                        x: 16
                        width: parent.width - 16
                        label: qsTr("Interactive seekbar")
                        description: qsTr("Allow clicking and dragging the seekbar to seek.")
                        value: settings.seekbarInteractive
                        onToggled: settings.seekbarInteractive = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Compact layout")
                        description: qsTr("Use a smaller, denser media flyout.")
                        value: settings.compactLayout
                        onToggled: settings.compactLayout = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Center title and artist")
                        description: qsTr("Center-align the track title and artist text.")
                        value: settings.centerTitleArtist
                        onToggled: settings.centerTitleArtist = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Always display")
                        description: qsTr("Keep the media flyout on screen instead of auto-hiding.")
                        value: settings.mediaFlyoutAlwaysDisplay
                        onToggled: settings.mediaFlyoutAlwaysDisplay = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Show player name")
                        description: qsTr("Show the name of the app playing the media.")
                        value: settings.playerInfoEnabled
                        onToggled: settings.playerInfoEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Accent from album art")
                        description: qsTr("Tint the flyout accents using the current album art's dominant color.")
                        value: settings.accentFromArt
                        onToggled: settings.accentFromArt = value
                    }
                    SettingRow {
                        visible: settings.accentFromArt
                        x: 16
                        width: parent.width - 16
                        label: qsTr("Apply accent app-wide")
                        description: qsTr("Also tint the dashboard and other windows, not just the flyouts.")
                        value: settings.accentFromArtAppWide
                        onToggled: settings.accentFromArtAppWide = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Repeat button")
                        description: qsTr("Show a repeat control on the media flyout.")
                        value: settings.repeatEnabled
                        onToggled: settings.repeatEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Shuffle button")
                        description: qsTr("Show a shuffle control on the media flyout.")
                        value: settings.shuffleEnabled
                        onToggled: settings.shuffleEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Now Playing flyout")
                        description: qsTr("Show a compact card with the next track when one auto-advances.")
                        available: capabilities.mediaSessionAvailable
                        unavailableNote: qsTr("Unavailable: no system media session was detected on this PC.")
                        value: settings.nextUpEnabled
                        onToggled: settings.nextUpEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Hide during fullscreen")
                        description: qsTr("Suppress flyouts while a fullscreen video game is in the foreground.")
                        value: settings.disableOnFullscreen
                        onToggled: settings.disableOnFullscreen = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Swipe to dismiss")
                        description: qsTr("Drag a flyout quickly in any direction, with the mouse or a finger, to close it.")
                        value: settings.flyoutSwipeDismissEnabled
                        onToggled: settings.flyoutSwipeDismissEnabled = value
                    }

                    SettingCombo {
                        width: parent.width
                        label: qsTr("Flyout position")
                        description: qsTr("Which side of the monitor flyouts appear on, or docked above the taskbar widget.")
                        model: [qsTr("Top left"), qsTr("Top center"), qsTr("Top right"), qsTr("Bottom left"), qsTr("Bottom center"), qsTr("Bottom right"), qsTr("Above taskbar widget")]
                        currentIndex: settings.flyoutPosition
                        onActivated: settings.flyoutPosition = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Monitor")
                        description: qsTr("Which display flyouts are shown on.")
                        model: monitors.names
                        currentIndex: settings.flyoutMonitor
                        onActivated: settings.flyoutMonitor = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: qsTr("Flyout duration")
                        description: qsTr("How long the media flyout stays on screen.")
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.flyoutDuration
                        valueText: qsTr("%1 s").arg((settings.flyoutDuration / 1000).toFixed(1))
                        onMoved: settings.flyoutDuration = Math.round(value)
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Animation speed")
                        description: qsTr("Flyout fade-in / fade-out speed.")
                        model: [qsTr("Off"), "0.5x", "1x", "1.5x", "2x", "3x"]
                        currentIndex: settings.flyoutAnimationSpeed
                        onActivated: settings.flyoutAnimationSpeed = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Animation easing")
                        description: qsTr("The easing curve used for flyout animations.")
                        model: [qsTr("Linear"), qsTr("Sine"), qsTr("Quadratic"), qsTr("Cubic")]
                        currentIndex: settings.flyoutAnimationEasingStyle
                        onActivated: settings.flyoutAnimationEasingStyle = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: qsTr("Now Playing duration")
                        description: qsTr("How long the now playing flyout stays on screen.")
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.nextUpDuration
                        valueText: qsTr("%1 s").arg((settings.nextUpDuration / 1000).toFixed(1))
                        onMoved: settings.nextUpDuration = Math.round(value)
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Now Playing style")
                        description: qsTr("How the now playing flyout is laid out.")
                        model: [qsTr("Now playing card"), qsTr("Compact pill")]
                        currentIndex: settings.nextUpStyle
                        onActivated: settings.nextUpStyle = index
                        locked: entitlement.locked.nowPlayingStyles
                    }
                }

                Column {
                    visible: dash.currentIndex === 5
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: qsTr("Volume flyout")
                        description: qsTr("Show the volume flyout on volume key presses.")
                        available: capabilities.audioAvailable
                        unavailableNote: qsTr("Unavailable: no audio output device was detected.")
                        value: settings.volumeFlyoutEnabled
                        onToggled: settings.volumeFlyoutEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Override native volume OSD")
                        description: qsTr("Replace the built-in Windows volume overlay with this one.")
                        value: settings.overrideNativeOsd
                        onToggled: settings.overrideNativeOsd = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("App mixer in volume flyout")
                        description: qsTr("Show a button on the volume flyout to expand a per-app volume mixer.")
                        value: settings.volumeFlyoutMixerEnabled
                        onToggled: settings.volumeFlyoutMixerEnabled = value
                        locked: entitlement.locked.perAppMixer
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Volume flyout position")
                        description: qsTr("Place the volume flyout independently of the media flyout.")
                        model: [qsTr("Top left"), qsTr("Top center"), qsTr("Top right"), qsTr("Bottom left"), qsTr("Bottom center"), qsTr("Bottom right"), qsTr("Above taskbar widget")]
                        currentIndex: settings.volumeFlyoutPosition
                        onActivated: settings.volumeFlyoutPosition = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Volume flyout monitor")
                        description: qsTr("Which display the volume flyout is shown on.")
                        model: monitors.names
                        currentIndex: settings.volumeFlyoutMonitor
                        onActivated: settings.volumeFlyoutMonitor = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: qsTr("Volume flyout duration")
                        description: qsTr("How long the volume flyout stays on screen.")
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.volumeFlyoutDuration
                        valueText: qsTr("%1 s").arg((settings.volumeFlyoutDuration / 1000).toFixed(1))
                        onMoved: settings.volumeFlyoutDuration = Math.round(value)
                    }
                }

                Column {
                    visible: dash.currentIndex === 6
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: qsTr("Lock key indicators")
                        description: qsTr("Show a flyout when Caps, Num, or Scroll lock is toggled.")
                        available: capabilities.lockKeysAvailable
                        unavailableNote: qsTr("Unavailable: the keyboard hook could not be installed.")
                        value: settings.lockKeysEnabled
                        onToggled: settings.lockKeysEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Caps Lock")
                        value: settings.lockKeysCapsEnabled
                        onToggled: settings.lockKeysCapsEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Num Lock")
                        value: settings.lockKeysNumEnabled
                        onToggled: settings.lockKeysNumEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Scroll Lock")
                        value: settings.lockKeysScrollEnabled
                        onToggled: settings.lockKeysScrollEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Insert")
                        value: settings.lockKeysInsertEnabled
                        onToggled: settings.lockKeysInsertEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Bold text")
                        description: qsTr("Use a bold font for the key name.")
                        value: settings.lockKeysBoldUi
                        onToggled: settings.lockKeysBoldUi = value
                    }
                    SettingSlider {
                        width: parent.width
                        label: qsTr("Duration")
                        description: qsTr("How long the lock keys flyout stays on screen.")
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.lockKeysDuration
                        valueText: qsTr("%1 s").arg((settings.lockKeysDuration / 1000).toFixed(1))
                        onMoved: settings.lockKeysDuration = Math.round(value)
                    }
                }

                Column {
                    id: taskbarWidgetPage
                    visible: dash.currentIndex === 4
                    width: parent.width
                    spacing: 12

                    // Polled: the taskbar edge has no change signal of its own.
                    property bool verticalTaskbar: false
                    Timer {
                        interval: 1000
                        running: taskbarWidgetPage.visible
                        repeat: true
                        triggeredOnStart: true
                        onTriggered: taskbarWidgetPage.verticalTaskbar = !taskbar.widgetFits(settings.taskbarWidgetMonitor)
                    }

                    Rectangle {
                        visible: taskbarWidgetPage.verticalTaskbar
                        width: parent.width
                        height: verticalNote.implicitHeight + 20
                        radius: 8
                        color: theme.card
                        border.width: 1
                        border.color: theme.border
                        Text {
                            id: verticalNote
                            anchors.fill: parent
                            anchors.margins: 10
                            wrapMode: Text.WordWrap
                            text: qsTr("The taskbar on the chosen display is on the left or right. The widgets need a horizontal taskbar (top or bottom) and stay hidden until it moves back.")
                            color: theme.textSecondary
                            font.pixelSize: 12
                        }
                    }

                    SettingCombo {
                        width: parent.width
                        label: qsTr("Widget side")
                        description: qsTr("Which part of the taskbar the widget docks to.")
                        model: [qsTr("Left"), qsTr("Center"), qsTr("Right")]
                        currentIndex: settings.taskbarWidgetSide
                        onActivated: settings.taskbarWidgetSide = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Widget layout")
                        description: settings.taskbarWidgetLayout === 1
                            ? qsTr("Media and peripheral widgets share one slot. Scroll over the widget to page between media, 2.4 GHz, Bluetooth and wired devices.")
                            : qsTr("Media and peripheral widgets sit next to each other on the taskbar.")
                        model: [qsTr("Side by side"), qsTr("Stack")]
                        currentIndex: settings.taskbarWidgetLayout
                        onActivated: settings.taskbarWidgetLayout = index
                        locked: entitlement.locked.taskbarWidgetOptions
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Widget monitor")
                        description: qsTr("Which display's taskbar the widget docks to.")
                        model: monitors.names
                        currentIndex: settings.taskbarWidgetMonitor
                        onActivated: settings.taskbarWidgetMonitor = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: qsTr("Widget offset")
                        description: settings.taskbarWidgetSide === 1
                            ? qsTr("Shift the centered widget left or right along the taskbar.")
                            : qsTr("Fine-tune the widget's horizontal position on the taskbar.")
                        from: settings.taskbarWidgetSide === 1 ? -600 : 0
                        to: 600
                        stepSize: 10
                        value: settings.taskbarWidgetOffset
                        valueText: qsTr("%1 px").arg(settings.taskbarWidgetOffset)
                        onMoved: settings.taskbarWidgetOffset = Math.round(value)
                    }

                    Column {
                        id: mediaWidgetSection
                        property bool expanded: true
                        width: parent.width
                        spacing: 8

                        Button {
                            id: mediaWidgetSectionButton
                            width: parent.width
                            height: 38
                            text: qsTr("Media Widget")
                            contentItem: Row {
                                spacing: 8
                                Text {
                                    text: mediaWidgetSection.expanded ? "-" : "+"
                                    color: theme.textSecondary
                                    font: mediaWidgetSectionButton.font
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Text {
                                    text: mediaWidgetSectionButton.text
                                    color: theme.textPrimary
                                    font: mediaWidgetSectionButton.font
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            onClicked: mediaWidgetSection.expanded = !mediaWidgetSection.expanded
                        }

                        Column {
                            visible: mediaWidgetSection.expanded
                            width: parent.width
                            spacing: 12

                            SettingRow {
                                width: parent.width
                                label: qsTr("Taskbar widget")
                                description: qsTr("Dock a compact now-playing widget on the taskbar near the tray.")
                                value: settings.taskbarWidgetEnabled
                                onToggled: settings.taskbarWidgetEnabled = value
                            }
                            SettingCombo {
                                width: parent.width
                                label: qsTr("Browser artwork")
                                description: qsTr("Use the website icon when album art is unavailable.")
                                model: ["Chrome", "Edge", "Firefox", "Brave"]
                                currentIndex: dash.browserIndex
                                onActivated: dash.browserIndex = index
                            }
                            Row {
                                spacing: 12
                                Button {
                                    text: qsTr("Install browser extension")
                                    enabled: browserIntegration.canInstall(dash.browserIds[dash.browserIndex])
                                    onClicked: browserIntegration.install(dash.browserIds[dash.browserIndex])
                                }
                                Button {
                                    visible: browserIntegration.developmentAvailable
                                    text: qsTr("Open development extension")
                                    onClicked: browserIntegration.openDevelopmentPackage(dash.browserIds[dash.browserIndex])
                                }
                            }
                            Text {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                color: theme.textTertiary
                                font.pixelSize: 12
                                text: browserIntegration.status !== "" ? browserIntegration.status
                                      : browserIntegration.canInstall(dash.browserIds[dash.browserIndex]) ? ""
                                      : qsTr("The browser extension is not published yet.")
                            }
                            SettingRow {
                                width: parent.width
                                label: qsTr("Widget transport controls")
                                description: qsTr("Show previous / play-pause / next buttons on the widget.")
                                value: settings.taskbarWidgetControlsEnabled
                                onToggled: settings.taskbarWidgetControlsEnabled = value
                                locked: entitlement.locked.taskbarWidgetOptions
                            }
                            SettingRow {
                                width: parent.width
                                label: qsTr("Pause all button")
                                description: qsTr("Show a button that pauses every playing media source at once.")
                                value: settings.taskbarWidgetPauseAllEnabled
                                onToggled: settings.taskbarWidgetPauseAllEnabled = value
                                locked: entitlement.locked.taskbarWidgetOptions
                            }
                            SettingRow {
                                visible: settings.taskbarWidgetPauseAllEnabled
                                x: 16
                                width: parent.width - 16
                                label: qsTr("Play all after pausing")
                                description: qsTr("When every source is paused, the same button becomes Play all and resumes them. When off, the button hides until something plays again.")
                                value: settings.taskbarWidgetPlayAllEnabled
                                onToggled: settings.taskbarWidgetPlayAllEnabled = value
                                locked: entitlement.locked.taskbarWidgetOptions
                            }
                            SettingRow {
                                width: parent.width
                                label: qsTr("Pause overlay")
                                description: qsTr("Dim the album art with a pause glyph when playback is paused.")
                                value: settings.taskbarWidgetShowPauseOverlay
                                onToggled: settings.taskbarWidgetShowPauseOverlay = value
                                locked: entitlement.locked.taskbarWidgetOptions
                            }
                            SettingRow {
                                width: parent.width
                                label: qsTr("Scroll long titles")
                                description: qsTr("Marquee-scroll the track title when it is too long to fit.")
                                value: settings.taskbarWidgetScrollingEnabled
                                onToggled: settings.taskbarWidgetScrollingEnabled = value
                                locked: entitlement.locked.taskbarWidgetOptions
                            }
                            SettingRow {
                                width: parent.width
                                label: qsTr("Adapt colors to the taskbar")
                                description: qsTr("Read the taskbar next to the widgets and switch between light and dark text. Keeps the widgets readable over a transparent taskbar or a live wallpaper.")
                                value: settings.taskbarWidgetAdaptiveColors
                                onToggled: settings.taskbarWidgetAdaptiveColors = value
                                locked: entitlement.locked.taskbarWidgetOptions
                            }
                            SettingRow {
                                width: parent.width
                                label: qsTr("Audio visualizer")
                                description: qsTr("Show a live spectrum of the system audio behind the widget.")
                                available: capabilities.audioAvailable
                                unavailableNote: qsTr("Unavailable: no audio output device was detected.")
                                value: settings.taskbarVisualizerEnabled
                                onToggled: settings.taskbarVisualizerEnabled = value
                                locked: entitlement.locked.visualizer
                            }
                            SettingCombo {
                                visible: settings.taskbarVisualizerEnabled
                                x: 16
                                width: parent.width - 16
                                label: qsTr("Visualizer style")
                                description: qsTr("How the audio is drawn behind the widget.")
                                model: [qsTr("Bars"), qsTr("Radial"), qsTr("Waveform"), qsTr("Particles")]
                                currentIndex: settings.visualizerStyle
                                onActivated: settings.visualizerStyle = index
                                locked: entitlement.locked.visualizer
                            }
                            SettingRow {
                                visible: settings.taskbarVisualizerEnabled && settings.visualizerStyle === 0
                                x: 16
                                width: parent.width - 16
                                label: qsTr("Mirror the equalizer")
                                description: qsTr("Grow the bars symmetrically from the middle instead of the bottom.")
                                value: settings.taskbarVisualizerCentered
                                onToggled: settings.taskbarVisualizerCentered = value
                                locked: entitlement.locked.visualizer
                            }
                        }
                    }

                    Column {
                        id: peripheralWidgetSection
                        property bool expanded: true
                        width: parent.width
                        spacing: 8

                        Button {
                            id: peripheralWidgetSectionButton
                            width: parent.width
                            height: 38
                            text: qsTr("Peripheral Widget")
                            contentItem: Row {
                                spacing: 8
                                Text {
                                    text: peripheralWidgetSection.expanded ? "-" : "+"
                                    color: theme.textSecondary
                                    font: peripheralWidgetSectionButton.font
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Text {
                                    text: peripheralWidgetSectionButton.text
                                    color: theme.textPrimary
                                    font: peripheralWidgetSectionButton.font
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            onClicked: peripheralWidgetSection.expanded = !peripheralWidgetSection.expanded
                        }

                        Column {
                            visible: peripheralWidgetSection.expanded
                            width: parent.width
                            spacing: 12

                            SettingRow {
                                width: parent.width
                                label: qsTr("Peripherals widget")
                                description: qsTr("Dock a battery widget for selected wireless peripherals on the taskbar.")
                                value: settings.peripheralsWidgetEnabled
                                onToggled: settings.peripheralsWidgetEnabled = value
                                locked: entitlement.locked.peripheralBattery
                            }
                            SettingRow {
                                visible: settings.peripheralsWidgetEnabled
                                width: parent.width
                                label: qsTr("Match app accent")
                                description: qsTr("Color the battery rings using the app accent (or album art, if that's enabled) instead of green/yellow/red by charge level.")
                                value: settings.batteryRingUseAccent
                                onToggled: settings.batteryRingUseAccent = value
                                parentLocked: entitlement.locked.peripheralBattery
                            }

                            Text {
                                visible: settings.peripheralsWidgetEnabled
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: qsTr("Choose which detected peripherals MediaFlyouts should show. Hidden devices are left out of the widget, the home page and the flyouts:")
                                color: theme.textSecondary
                                font.pixelSize: 12
                                topPadding: 4
                            }
                            Text {
                                visible: settings.peripheralsWidgetEnabled && dash.wirelessDevices.length === 0
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: qsTr("No peripherals are currently connected.")
                                color: theme.textMuted
                                font.pixelSize: 12
                            }
                            Repeater {
                                model: settings.peripheralsWidgetEnabled ? dash.wirelessDevices : []
                                delegate: PeripheralRow {
                                    required property var modelData
                                    width: parent.width
                                    deviceName: modelData.name
                                    description: {
                                        var text = modelData.percent >= 0 ? qsTr("Currently at %1%").arg(modelData.percent) : qsTr("Battery level unknown")
                                        var parts = []
                                        for(var i = 0; i < modelData.components.length; i++)
                                            parts.push(modelData.components[i].label + " " + modelData.components[i].percent + "%")
                                        if(parts.length > 1)
                                            text += " (" + parts.join(", ") + ")"
                                        var link = DeviceIcons.transport(modelData.connectionKind)
                                        return link ? qsTr("%1, over %2.").arg(text).arg(link) : text + "."
                                    }
                                    value: settings.peripheralSelected(modelData.name)
                                    onToggled: settings.setPeripheralSelected(modelData.name, value)
                                    parentLocked: entitlement.locked.peripheralBattery
                                }
                            }

                            Rectangle {
                                visible: settings.peripheralsWidgetEnabled
                                width: parent.width
                                height: 1
                                color: theme.border
                            }
                            SettingRow {
                                visible: settings.peripheralsWidgetEnabled
                                width: parent.width
                                label: qsTr("Wired devices")
                                description: qsTr("List plugged-in USB peripherals.")
                                value: settings.peripheralsWidgetWiredEnabled
                                onToggled: settings.peripheralsWidgetWiredEnabled = value
                                parentLocked: entitlement.locked.peripheralBattery
                            }
                            Text {
                                visible: settings.peripheralsWidgetEnabled && settings.peripheralsWidgetWiredEnabled
                                         && dash.wiredDevices.length === 0
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: qsTr("No wired peripherals are currently plugged in.")
                                color: theme.textMuted
                                font.pixelSize: 12
                            }
                            Repeater {
                                model: settings.peripheralsWidgetEnabled && settings.peripheralsWidgetWiredEnabled
                                       ? dash.wiredDevices : []
                                delegate: PeripheralRow {
                                    required property var modelData
                                    width: parent.width
                                    deviceName: modelData.name
                                    description: qsTr("Plugged in over USB.")
                                    value: settings.peripheralSelected(modelData.name)
                                    onToggled: settings.setPeripheralSelected(modelData.name, value)
                                    parentLocked: entitlement.locked.peripheralBattery
                                }
                            }

                        }
                    }
                }

                Column {
                    visible: dash.currentIndex === 2
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: qsTr("Peripheral flyout")
                        description: qsTr("Show a flyout when a peripheral connects, disconnects or runs low on battery.")
                        value: settings.peripheralFlyoutEnabled
                        onToggled: settings.peripheralFlyoutEnabled = value
                        locked: entitlement.locked.peripheralBattery
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("On connect")
                        description: qsTr("Announce a peripheral when it connects.")
                        value: settings.peripheralFlyoutOnConnect
                        onToggled: settings.peripheralFlyoutOnConnect = value
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("On disconnect")
                        description: qsTr("Announce a peripheral when it disconnects.")
                        value: settings.peripheralFlyoutOnDisconnect
                        onToggled: settings.peripheralFlyoutOnDisconnect = value
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("On low battery")
                        description: qsTr("Warn once when a battery drops below 20%, and again below 10%.")
                        value: settings.peripheralFlyoutOnLowBattery
                        onToggled: settings.peripheralFlyoutOnLowBattery = value
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("On charging")
                        description: qsTr("Announce when a peripheral starts or stops charging.")
                        value: settings.peripheralFlyoutOnCharging
                        onToggled: settings.peripheralFlyoutOnCharging = value
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Wired devices")
                        description: qsTr("Announce USB peripherals as they are plugged in or removed.")
                        value: settings.peripheralFlyoutOnWired
                        onToggled: settings.peripheralFlyoutOnWired = value
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Low battery notification")
                        description: qsTr("Also show a tray notification for a low battery, so it isn't missed.")
                        value: settings.peripheralLowBatteryToast
                        onToggled: settings.peripheralLowBatteryToast = value
                        locked: entitlement.locked.peripheralBattery
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Hide during fullscreen")
                        description: qsTr("Suppress flyouts while a fullscreen video game is in the foreground. Shared with the other flyouts.")
                        value: settings.disableOnFullscreen
                        onToggled: settings.disableOnFullscreen = value
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Peripheral flyout position")
                        description: qsTr("Place the peripheral flyout independently of the other flyouts.")
                        model: [qsTr("Top left"), qsTr("Top center"), qsTr("Top right"), qsTr("Bottom left"), qsTr("Bottom center"), qsTr("Bottom right"), qsTr("Above taskbar widget")]
                        currentIndex: settings.peripheralFlyoutPosition
                        onActivated: settings.peripheralFlyoutPosition = index
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Peripheral flyout monitor")
                        description: qsTr("Which display the peripheral flyout is shown on.")
                        model: monitors.names
                        currentIndex: settings.peripheralFlyoutMonitor
                        onActivated: settings.peripheralFlyoutMonitor = index
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                    SettingSlider {
                        width: parent.width
                        label: qsTr("Peripheral flyout duration")
                        description: qsTr("How long the peripheral flyout stays on screen.")
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.peripheralFlyoutDuration
                        valueText: qsTr("%1 s").arg((settings.peripheralFlyoutDuration / 1000).toFixed(1))
                        onMoved: settings.peripheralFlyoutDuration = Math.round(value)
                        parentLocked: entitlement.locked.peripheralBattery
                    }
                }

                Column {
                    visible: dash.currentIndex === 3
                    width: parent.width
                    spacing: 12

                    SettingRow {
                        width: parent.width
                        label: qsTr("Charger flyout")
                        description: qsTr("Show a flyout when the charger is plugged in or unplugged.")
                        available: capabilities.chargerAvailable
                        unavailableNote: qsTr("Unavailable: this PC has no battery, so it never changes power source.")
                        value: settings.chargerFlyoutEnabled
                        onToggled: settings.chargerFlyoutEnabled = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("On plug in")
                        description: qsTr("Announce when the charger is connected.")
                        available: capabilities.chargerAvailable
                        value: settings.chargerFlyoutOnPlug
                        onToggled: settings.chargerFlyoutOnPlug = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("On unplug")
                        description: qsTr("Announce when the charger is disconnected.")
                        available: capabilities.chargerAvailable
                        value: settings.chargerFlyoutOnUnplug
                        onToggled: settings.chargerFlyoutOnUnplug = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Hide during fullscreen")
                        description: qsTr("Suppress flyouts while a fullscreen video game is in the foreground. Shared with the other flyouts.")
                        value: settings.disableOnFullscreen
                        onToggled: settings.disableOnFullscreen = value
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Charger flyout position")
                        description: qsTr("Place the charger flyout independently of the other flyouts.")
                        model: [qsTr("Top left"), qsTr("Top center"), qsTr("Top right"), qsTr("Bottom left"), qsTr("Bottom center"), qsTr("Bottom right"), qsTr("Above taskbar widget")]
                        currentIndex: settings.chargerFlyoutPosition
                        onActivated: settings.chargerFlyoutPosition = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Charger flyout monitor")
                        description: qsTr("Which display the charger flyout is shown on.")
                        model: monitors.names
                        currentIndex: settings.chargerFlyoutMonitor
                        onActivated: settings.chargerFlyoutMonitor = index
                    }
                    SettingSlider {
                        width: parent.width
                        label: qsTr("Charger flyout duration")
                        description: qsTr("How long the charger flyout stays on screen.")
                        from: 1000
                        to: 10000
                        stepSize: 500
                        value: settings.chargerFlyoutDuration
                        valueText: qsTr("%1 s").arg((settings.chargerFlyoutDuration / 1000).toFixed(1))
                        onMoved: settings.chargerFlyoutDuration = Math.round(value)
                    }
                }

                Column {
                    visible: dash.currentIndex === 7
                    width: parent.width
                    spacing: 12

                    SettingCombo {
                        width: parent.width
                        label: qsTr("Filter mode")
                        description: qsTr("Off shows every app. Blacklist hides the listed apps; whitelist shows only the listed apps.")
                        model: [qsTr("Off"), qsTr("Blacklist (hide these)"), qsTr("Whitelist (show only these)")]
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
                                ? qsTr("App name (e.g. %1)").arg(nowPlaying.playerName)
                                : qsTr("App name (e.g. Spotify, chrome)")
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
                            text: qsTr("Add")
                            onClicked: { settings.addFilterEntry(filterInput.text); filterInput.text = "" }
                        }
                    }

                    Text {
                        visible: settings.filterMode !== 0 && nowPlaying.playerName.length > 0
                        width: parent.width
                        text: qsTr("Now playing app: %1").arg(nowPlaying.playerName)
                        color: theme.textMuted
                        font.pixelSize: 12
                    }

                    Column {
                        visible: settings.filterMode !== 0
                        width: parent.width
                        spacing: 6

                        Text {
                            visible: settings.filterList.length === 0
                            text: qsTr("No apps added yet.")
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

                Column {
                    visible: dash.currentIndex === 8
                    width: parent.width
                    spacing: 12

                    SettingCombo {
                        id: languageCombo
                        width: parent.width
                        label: qsTr("Language")
                        description: qsTr("Pick the language for flyouts, widgets and this dashboard.")
                        // Codes stay fixed. Only the first label is translated: it names the system default.
                        readonly property var languageCodes: i18n.languages.map(function(entry) { return entry.code })
                        model: i18n.languages.map(function(entry) {
                            return entry.code === "" ? qsTr("System default") : entry.name
                        })
                        currentIndex: Math.max(0, languageCodes.indexOf(settings.language))
                        onActivated: settings.language = languageCodes[index]
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Theme")
                        description: qsTr("Match the system theme, or force light or dark.")
                        model: [qsTr("Use system setting"), qsTr("Light"), qsTr("Dark")]
                        currentIndex: settings.appTheme
                        onActivated: settings.appTheme = index
                    }
                    SettingCombo {
                        width: parent.width
                        label: qsTr("Accent color")
                        description: qsTr("Use the built-in accent, match the Windows accent, or pick a custom color.")
                        model: [qsTr("Default"), qsTr("Match system"), qsTr("Custom")]
                        currentIndex: settings.accentMode
                        onActivated: settings.accentMode = index
                    }
                    SettingCombo {
                        // Only a licence holder has a Pro mark to choose.
                        visible: entitlement.licensed
                        width: parent.width
                        label: qsTr("Pro icon")
                        description: qsTr("Which Pro mark the tray, window frames and this dashboard show.")
                        model: [qsTr("Side bar"), qsTr("Corner ribbon")]
                        currentIndex: settings.proIconStyle
                        onActivated: settings.proIconStyle = index
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
                            text: qsTr("Choose color…")
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: {
                                accentDialog.selectedColor = theme.accent
                                accentDialog.open()
                            }
                        }
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Launch on startup")
                        description: qsTr("Start MediaFlyouts automatically when you sign in.")
                        value: settings.launchOnStartup
                        onToggled: settings.launchOnStartup = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Tray left-click shows media flyout")
                        description: qsTr("Left-clicking the tray icon shows the media flyout instead of the dashboard.")
                        value: settings.trayLeftClickShowsFlyout
                        onToggled: settings.trayLeftClickShowsFlyout = value
                    }
                    SettingRow {
                        width: parent.width
                        label: qsTr("Hide tray icon")
                        description: qsTr("Hide the notification-area icon (re-launch the app to reopen the dashboard).")
                        value: settings.trayIconHidden
                        onToggled: settings.trayIconHidden = value
                    }

                    Row {
                        spacing: 12
                        topPadding: 8
                        Button {
                            text: qsTr("Export settings…")
                            onClicked: exportDialog.open()
                        }
                        Button {
                            text: qsTr("Import settings…")
                            onClicked: importDialog.open()
                        }
                    }

                    Text {
                        text: qsTr("Restore defaults")
                        color: theme.textPrimary
                        font.pixelSize: 15
                        font.bold: true
                        topPadding: 12
                    }
                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: qsTr("Puts every setting back to its factory value, including flyout positions, widget options, player filters and peripheral names. Export your settings first if you may want them back. The first-run wizard does not show again.")
                        color: theme.textSecondary
                        font.pixelSize: 13
                    }
                    Row {
                        spacing: 12
                        Button {
                            text: qsTr("Restore default settings…")
                            onClicked: resetConfirm.open()
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: dash.resetStatus
                            color: theme.textTertiary
                            font.pixelSize: 12
                        }
                    }
                }

                SupportPage {
                    visible: dash.currentIndex === 9 && donate.available
                    width: parent.width
                    ownerWindow: dash
                }

                Column {
                    visible: dash.currentIndex === 10
                    width: parent.width
                    spacing: 12

                    Row {
                        spacing: 12
                        AppLogo {
                            anchors.verticalCenter: parent.verticalCenter
                            size: 40
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
                                text: qsTr("Version %1").arg(updateChecker.currentVersion) + " · " + updateChecker.installKindLabel
                                color: theme.textMuted
                                font.pixelSize: 12
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: qsTr("Fluent-style media, volume, lock-key and peripheral overlays for Windows, built with Qt 6.")
                        color: theme.textSecondary
                        font.pixelSize: 13
                        topPadding: 4
                    }

                    Row {
                        spacing: 12
                        topPadding: 8
                        Button {
                            text: qsTr("Check for updates")
                            enabled: !updateChecker.checking
                            onClicked: {
                                settings.skippedUpdateVersion = ""
                                updateChecker.check()
                            }
                        }
                        Button {
                            visible: updateChecker.updateAvailable
                            text: qsTr("Get %1").arg(updateChecker.latestVersion)
                            onClicked: updateChecker.notifyUpdate()
                        }
                        Button {
                            text: qsTr("What's new")
                            onClicked: whatsNew.requestShow()
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: updateChecker.status
                            color: updateChecker.updateAvailable ? theme.accent : theme.textTertiary
                            font.pixelSize: 13
                        }
                    }

                    Text {
                        visible: updateChecker.updateAvailable
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: updateChecker.installKind === "store"
                              ? qsTr("This copy updates through the Microsoft Store.")
                              : updateChecker.installKind === "portable"
                              ? qsTr("Portable version: download the new ZIP from the release page and replace the app folder.")
                              : qsTr("The installer downloads inside the app. Click Install when it is ready. Your settings are kept.")
                        color: theme.textTertiary
                        font.pixelSize: 12
                    }

                    SettingRow {
                        width: parent.width
                        label: qsTr("Check for updates on launch")
                        description: qsTr("Automatically look for a newer release shortly after startup.")
                        value: settings.autoCheckUpdates
                        onToggled: settings.autoCheckUpdates = value
                    }

                    Text {
                        text: qsTr("Report a problem")
                        color: theme.textPrimary
                        font.pixelSize: 15
                        font.bold: true
                        topPadding: 12
                    }
                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: qsTr("Create a report with the app log, your settings and system details, then attach it to a new GitHub issue. The issue page opens with a blank template. Nothing is sent until you post the issue. You can open the report and check its contents first.")
                        color: theme.textSecondary
                        font.pixelSize: 13
                    }
                    Row {
                        spacing: 12
                        Button {
                            text: qsTr("Create error report")
                            onClicked: errorReporter.createReport()
                        }
                        Button {
                            visible: errorReporter.lastReportPath !== ""
                            text: qsTr("Show report")
                            onClicked: errorReporter.revealReport()
                        }
                        Button {
                            visible: errorReporter.lastReportPath !== ""
                            text: qsTr("Open new issue")
                            onClicked: errorReporter.openIssuePage()
                        }
                    }
                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: errorReporter.lastReportPath !== ""
                              ? errorReporter.status + " " + errorReporter.lastReportPath
                              : errorReporter.status
                        color: theme.textTertiary
                        font.pixelSize: 12
                    }

                    Text {
                        text: qsTr("Restart")
                        color: theme.textPrimary
                        font.pixelSize: 15
                        font.bold: true
                        topPadding: 12
                    }
                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: qsTr("Saves an error report, then closes and reopens the app. Use it when a flyout or widget looks wrong: the report keeps the log of this session so the problem can be traced after the restart.")
                        color: theme.textSecondary
                        font.pixelSize: 13
                    }
                    Row {
                        spacing: 12
                        Button {
                            text: qsTr("Save report and restart")
                            onClicked: restarter.restart()
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: restarter.status
                            color: theme.textTertiary
                            font.pixelSize: 12
                        }
                    }

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 8
                        color: repoHover.containsMouse ? theme.cardHover : theme.card
                        border.color: theme.border

                        Image {
                            anchors.centerIn: parent
                            width: 18
                            height: 18
                            source: theme.dark
                                     ? "qrc:/qt/qml/MediaFlyouts/Views/assets/github-mark-light.svg"
                                     : "qrc:/qt/qml/MediaFlyouts/Views/assets/github-mark-dark.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            mipmap: true
                        }
                        MouseArea {
                            id: repoHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://github.com/lalitmeena04/MediaFlyouts-Releases")
                        }
                    }
                }
                }
            }
        }
    }

    FileDialog {
        id: exportDialog
        title: qsTr("Export settings")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("Settings files (*.ini)")]
        defaultSuffix: "ini"
        onAccepted: settings.exportTo(selectedFile)
    }
    FileDialog {
        id: importDialog
        title: qsTr("Import settings")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Settings files (*.ini)")]
        onAccepted: settings.importFrom(selectedFile)
    }
    ColorDialog {
        id: accentDialog
        title: qsTr("Choose accent color")
        onAccepted: settings.customAccent = selectedColor
    }

    property string resetStatus: ""
    Timer {
        id: resetStatusTimer
        interval: 4000
        onTriggered: dash.resetStatus = ""
    }
    function resetToDefaults() {
        log.info("Dashboard: user restored default settings")
        settings.resetToDefaults()
        dash.resetStatus = qsTr("Defaults restored.")
        resetStatusTimer.restart()
    }

    // Reset drops every stored value. Ask first.
    Popup {
        id: resetConfirm
        anchors.centerIn: parent
        width: 380
        modal: true
        focus: true
        padding: 20
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            radius: 8
            color: theme.card
            border.color: theme.border
        }
        contentItem: Column {
            spacing: 12
            Text {
                text: qsTr("Restore default settings?")
                color: theme.textPrimary
                font.pixelSize: 15
                font.bold: true
            }
            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("All settings go back to their factory values. This cannot be undone.")
                color: theme.textSecondary
                font.pixelSize: 13
            }
            Row {
                anchors.right: parent.right
                spacing: 8
                topPadding: 4
                Button {
                    text: qsTr("Cancel")
                    onClicked: resetConfirm.close()
                }
                Button {
                    text: qsTr("Restore defaults")
                    onClicked: {
                        resetConfirm.close()
                        dash.resetToDefaults()
                    }
                }
            }
        }
    }
}
