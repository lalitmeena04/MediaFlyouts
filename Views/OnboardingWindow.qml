import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import MediaFlyouts

// First-run wizard that introduces the app and captures a few key preferences.
Window {
    id: root
    width: 520
    height: 460
    minimumWidth: 460
    minimumHeight: 430
    title: "Welcome to MediaFlyouts"
    color: theme.window
    flags: Qt.Dialog
    visible: false

    Component.onCompleted: windowEffects.setDarkFrame(root, theme.dark)

    property int step: 0
    readonly property int stepCount: 4

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
            IconLabel {
                glyph: Fluent.musicNote
                size: 28
                color: theme.accent
            }
            Text {
                text: "MediaFlyouts"
                color: theme.textPrimary
                font.pixelSize: 22
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // step content, bounded by the header and footer so it can never overlap them
        Item {
            anchors.top: header.bottom
            anchors.topMargin: 18
            anchors.bottom: footer.top
            anchors.bottomMargin: 18
            anchors.left: parent.left
            anchors.right: parent.right
            clip: true

            // 0: welcome
            Column {
                visible: root.step === 0
                width: parent.width
                spacing: 12
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: "MediaFlyouts shows sleek Fluent-style overlays for your media, volume and lock keys — plus a taskbar widget and per-app volume mixer."
                    color: theme.textSecondary
                    font.pixelSize: 15
                }
                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: "Let's set up a few things. You can change everything later from the dashboard."
                    color: theme.textTertiary
                    font.pixelSize: 13
                }
            }

            // 1: theme
            Column {
                visible: root.step === 1
                width: parent.width
                spacing: 14
                Text {
                    text: "Choose a theme"
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }
                SettingCombo {
                    width: parent.width
                    label: "Theme"
                    description: "Match the system, or force light or dark."
                    model: ["Use system setting", "Light", "Dark"]
                    currentIndex: settings.appTheme
                    onActivated: settings.appTheme = index
                }
            }

            // 2: features
            Column {
                visible: root.step === 2
                width: parent.width
                spacing: 6
                Text {
                    text: "Pick your overlays"
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }
                SettingRow {
                    width: parent.width
                    label: "Media flyout"
                    description: "Show now-playing info on media keys."
                    value: settings.mediaFlyoutEnabled
                    onToggled: settings.mediaFlyoutEnabled = value
                }
                SettingRow {
                    width: parent.width
                    label: "Volume flyout"
                    description: "Replace the native volume popup."
                    value: settings.volumeFlyoutEnabled
                    onToggled: settings.volumeFlyoutEnabled = value
                }
                SettingRow {
                    width: parent.width
                    label: "Lock keys flyout"
                    description: "Show Caps / Num / Scroll indicators."
                    value: settings.lockKeysEnabled
                    onToggled: settings.lockKeysEnabled = value
                }
            }

            // 3: startup
            Column {
                visible: root.step === 3
                width: parent.width
                spacing: 6
                Text {
                    text: "Almost done"
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
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
                    label: "Taskbar widget"
                    description: "Dock a compact now-playing widget on the taskbar."
                    value: settings.taskbarWidgetEnabled
                    onToggled: settings.taskbarWidgetEnabled = value
                }
            }
        }

        // pinned to the bottom; the step content sizes itself around it
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
                    text: "Back"
                    visible: root.step > 0
                    onClicked: root.step--
                }
                Button {
                    id: nextBtn
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.step === root.stepCount - 1 ? "Get started" : "Next"
                    onClicked: root.step === root.stepCount - 1 ? root.finish() : root.step++
                }
            }
        }
    }

    onClosing: settings.onboardingCompleted = true
}
