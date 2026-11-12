import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import MediaFlyouts

Window {
    id: root
    width: 520
    height: 540
    minimumWidth: 460
    minimumHeight: 500
    title: qsTr("Welcome to MediaFlyouts")
    color: theme.window
    flags: Qt.Dialog
    visible: false

    Component.onCompleted: windowEffects.setDarkFrame(root, theme.dark)

    property int step: 0
    property int browserIndex: 0
    readonly property int stepCount: 5
    readonly property var browserIds: ["chrome", "msedge", "firefox", "brave"]

    function finish() {
        settings.onboardingCompleted = true
        root.close()
    }

    Item {
        anchors.fill: parent
        anchors.margins: 28

        Row {
            id: header
            anchors.top: parent.top
            anchors.left: parent.left
            spacing: 12
            AppLogo {
                size: 28
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: "MediaFlyouts"
                color: theme.textPrimary
                font.pixelSize: 22
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Item {
            anchors.top: header.bottom
            anchors.topMargin: 18
            anchors.bottom: footer.top
            anchors.bottomMargin: 18
            anchors.left: parent.left
            anchors.right: parent.right
            clip: true

            Column {
                visible: root.step === 0
                width: parent.width
                spacing: 12
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: qsTr("MediaFlyouts shows sleek Fluent-style overlays for your media, volume and lock keys — plus a taskbar widget and per-app volume mixer.")
                    color: theme.textSecondary
                    font.pixelSize: 15
                }
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: qsTr("Let's set up a few things. You can change everything later from the dashboard.")
                    color: theme.textTertiary
                    font.pixelSize: 13
                }
            }

            Column {
                visible: root.step === 1
                width: parent.width
                spacing: 14
                Text {
                    text: qsTr("Choose a language and theme")
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }
                SettingCombo {
                    width: parent.width
                    label: qsTr("Language")
                    description: qsTr("Follow the system language, or pick one.")
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
                    description: qsTr("Match the system, or force light or dark.")
                    model: [qsTr("Use system setting"), qsTr("Light"), qsTr("Dark")]
                    currentIndex: settings.appTheme
                    onActivated: settings.appTheme = index
                }
            }

            Column {
                visible: root.step === 2
                width: parent.width
                spacing: 6
                Text {
                    text: qsTr("Pick your overlays")
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }
                SettingRow {
                    width: parent.width
                    label: qsTr("Media flyout")
                    description: qsTr("Show now-playing info on media keys.")
                    available: capabilities.mediaSessionAvailable
                    unavailableNote: qsTr("Unavailable: no system media session was detected.")
                    value: settings.mediaFlyoutEnabled
                    onToggled: settings.mediaFlyoutEnabled = value
                }
                SettingRow {
                    width: parent.width
                    label: qsTr("Volume flyout")
                    description: qsTr("Replace the native volume popup.")
                    available: capabilities.audioAvailable
                    unavailableNote: qsTr("Unavailable: no audio output device was detected.")
                    value: settings.volumeFlyoutEnabled
                    onToggled: settings.volumeFlyoutEnabled = value
                }
                SettingRow {
                    width: parent.width
                    label: qsTr("Lock keys flyout")
                    description: qsTr("Show Caps / Num / Scroll indicators.")
                    available: capabilities.lockKeysAvailable
                    unavailableNote: qsTr("Unavailable: the keyboard hook could not be installed.")
                    value: settings.lockKeysEnabled
                    onToggled: settings.lockKeysEnabled = value
                }
            }

            Column {
                visible: root.step === 3
                width: parent.width
                spacing: 6
                Text {
                    text: qsTr("Peripherals and power")
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }
                SettingRow {
                    width: parent.width
                    label: qsTr("Peripheral flyout")
                    description: qsTr("Show a flyout when a wireless device connects, disconnects or runs low on battery.")
                    value: settings.peripheralFlyoutEnabled
                    onToggled: settings.peripheralFlyoutEnabled = value
                }
                SettingRow {
                    width: parent.width
                    label: qsTr("Peripherals widget")
                    description: qsTr("Dock a battery widget for your wireless devices on the taskbar.")
                    value: settings.peripheralsWidgetEnabled
                    onToggled: settings.peripheralsWidgetEnabled = value
                }
                SettingRow {
                    width: parent.width
                    label: qsTr("Charger flyout")
                    description: qsTr("Show a flyout when the charger is plugged in or unplugged.")
                    available: capabilities.chargerAvailable
                    unavailableNote: qsTr("Unavailable: this PC has no battery, so it never changes power source.")
                    value: settings.chargerFlyoutEnabled
                    onToggled: settings.chargerFlyoutEnabled = value
                }
            }

            Column {
                visible: root.step === 4
                width: parent.width
                spacing: 6
                Text {
                    text: qsTr("Almost done")
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
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
                    label: qsTr("Taskbar widget")
                    description: qsTr("Dock a compact now-playing widget on the taskbar.")
                    value: settings.taskbarWidgetEnabled
                    onToggled: settings.taskbarWidgetEnabled = value
                }
                SettingCombo {
                    width: parent.width
                    label: qsTr("Browser artwork")
                    description: qsTr("Use the website icon when album art is unavailable.")
                    model: ["Chrome", "Edge", "Firefox", "Brave"]
                    currentIndex: root.browserIndex
                    onActivated: root.browserIndex = index
                }
                Row {
                    spacing: 12
                    Button {
                        text: qsTr("Install browser extension")
                        enabled: browserIntegration.canInstall(root.browserIds[root.browserIndex])
                        onClicked: browserIntegration.install(root.browserIds[root.browserIndex])
                    }
                }
                Text {
                    width: parent.width
                    visible: browserIntegration.status !== ""
                    wrapMode: Text.WordWrap
                    color: theme.textTertiary
                    font.pixelSize: 12
                    text: browserIntegration.status
                }
                SettingRow {
                    width: parent.width
                    label: qsTr("Check for updates on launch")
                    description: qsTr("Look for a newer release shortly after startup. Updates install from inside the app.")
                    value: settings.autoCheckUpdates
                    onToggled: settings.autoCheckUpdates = value
                }
            }
        }

        Column {
            id: footer
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 18

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 6
                Repeater {
                    model: root.stepCount
                    delegate: Rectangle {
                        required property int index
                        width: 8
                        height: 8
                        radius: 4
                        color: index === root.step ? theme.accent : theme.border
                    }
                }
            }

            Item {
                width: parent.width
                height: nextBtn.height
                Button {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Back")
                    visible: root.step > 0
                    onClicked: root.step--
                }
                Button {
                    id: nextBtn
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.step === root.stepCount - 1 ? qsTr("Get started") : qsTr("Next")
                    onClicked: root.step === root.stepCount - 1 ? root.finish() : root.step++
                }
            }
        }
    }

    onClosing: settings.onboardingCompleted = true
}
