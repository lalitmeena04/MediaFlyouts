import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import MediaFlyouts

// Asks the user to skip or install a newer release. The installer downloads in-app.
Window {
    id: root
    width: 460
    height: 280
    minimumWidth: 420
    minimumHeight: 260
    title: "Update available"
    color: theme.window
    flags: Qt.Dialog
    visible: false

    Component.onCompleted: windowEffects.setDarkFrame(root, theme.dark)

    function open() {
        root.show()
        root.raise()
        root.requestActivate()
    }

    // Remember the skipped tag. Auto-check stays quiet for it.
    function skip() {
        settings.skippedUpdateVersion = updateChecker.latestVersion
        root.close()
    }

    readonly property bool storeInstall: updateChecker.installKind === "store"
    readonly property bool portableInstall: updateChecker.installKind === "portable"

    function primaryAction() {
        if(root.storeInstall) {
            updateChecker.openStorePage()
            return
        }
        if(!updateChecker.installerAvailable) {
            updateChecker.openReleasePage()
            return
        }
        if(updateChecker.downloaded)
            updateChecker.install()
        else
            updateChecker.download()
    }

    readonly property string primaryText: root.storeInstall ? "Open Microsoft Store"
                                        : !updateChecker.installerAvailable ? "Open download page"
                                        : updateChecker.downloaded ? "Install"
                                        : updateChecker.downloading ? "Downloading…"
                                        : "Download & install"

    // Explains the path this copy takes to the new version.
    readonly property string routeText: root.storeInstall
                                        ? "This copy came from the Microsoft Store. The Store delivers the update; open it to install now."
                                        : root.portableInstall
                                        ? "You run the portable version. Download the new ZIP from the release page and replace the app folder."
                                        : !updateChecker.installerAvailable
                                        ? "This release has no installer file. Get it from the release page instead."
                                        : ""

    readonly property string progressText: updateChecker.downloadError !== "" ? updateChecker.downloadError
                                         : updateChecker.downloaded ? "Download complete. Click Install to continue."
                                         : updateChecker.downloading ? "Downloading… " + Math.round(updateChecker.downloadProgress * 100) + "%"
                                         : ""

    // Closing mid-download stops it. Nothing is kept.
    onClosing: if(updateChecker.downloading) updateChecker.cancelDownload()

    Item {
        anchors.fill: parent
        anchors.margins: 24

        Column {
            id: body
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 12

            Row {
                spacing: 12
                IconLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    glyph: Fluent.musicNote
                    size: 28
                    color: theme.accent
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "MediaFlyouts " + updateChecker.latestVersion + " is available"
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }
            }

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: updateChecker.installerAvailable
                      ? "You have version " + updateChecker.currentVersion + ". The installer runs after the download. Your settings are kept."
                      : "You have version " + updateChecker.currentVersion + ". Your settings are kept."
                color: theme.textSecondary
                font.pixelSize: 13
            }

            Text {
                visible: root.routeText !== ""
                width: parent.width
                wrapMode: Text.WordWrap
                text: root.routeText
                color: theme.textTertiary
                font.pixelSize: 12
            }

            Rectangle {
                id: progressBar
                visible: updateChecker.downloading || updateChecker.downloaded
                width: parent.width
                height: 6
                radius: 3
                color: theme.border
                Rectangle {
                    width: parent.width * updateChecker.downloadProgress
                    height: parent.height
                    radius: 3
                    color: theme.accent
                    Behavior on width { NumberAnimation { duration: 120 } }
                }
            }

            Text {
                visible: root.progressText !== ""
                width: parent.width
                wrapMode: Text.WordWrap
                text: root.progressText
                color: updateChecker.downloadError !== "" ? theme.textPrimary : theme.textTertiary
                font.pixelSize: 12
            }
        }

        Row {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            spacing: 8
            Button {
                text: "Skip this version"
                enabled: !updateChecker.downloading
                onClicked: root.skip()
            }
            Button {
                text: root.primaryText
                enabled: !updateChecker.downloading
                onClicked: root.primaryAction()
            }
        }
    }
}
