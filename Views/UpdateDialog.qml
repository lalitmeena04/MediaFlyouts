import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import MediaFlyouts

Window {
    id: root
    width: 460
    height: 280
    minimumWidth: 420
    minimumHeight: 260
    title: qsTr("Update available")
    color: theme.window
    flags: Qt.Dialog
    visible: false

    Component.onCompleted: windowEffects.setDarkFrame(root, theme.dark)

    function open() {
        root.show()
        root.raise()
        root.requestActivate()
    }

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

    readonly property string primaryText: root.storeInstall ? qsTr("Open Microsoft Store")
                                        : !updateChecker.installerAvailable ? qsTr("Open download page")
                                        : updateChecker.downloaded ? qsTr("Install")
                                        : updateChecker.downloading ? qsTr("Downloading…")
                                        : qsTr("Download & install")

    readonly property string routeText: root.storeInstall
                                        ? qsTr("This copy came from the Microsoft Store. The Store delivers the update; open it to install now.")
                                        : root.portableInstall
                                        ? qsTr("You run the portable version. Download the new ZIP from the release page and replace the app folder.")
                                        : !updateChecker.installerAvailable
                                        ? qsTr("This release has no installer file. Get it from the release page instead.")
                                        : ""

    readonly property string progressText: updateChecker.downloadError !== "" ? updateChecker.downloadError
                                         : updateChecker.downloaded ? qsTr("Download complete. Click Install to continue.")
                                         : updateChecker.downloading ? qsTr("Downloading… %1%").arg(Math.round(updateChecker.downloadProgress * 100))
                                         : ""

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
                AppLogo {
                    anchors.verticalCenter: parent.verticalCenter
                    size: 28
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("MediaFlyouts %1 is available").arg(updateChecker.latestVersion)
                    color: theme.textPrimary
                    font.pixelSize: 17
                    font.bold: true
                }
            }

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: updateChecker.installerAvailable
                      ? qsTr("You have version %1. The installer runs after the download. Your settings are kept.").arg(updateChecker.currentVersion)
                      : qsTr("You have version %1. Your settings are kept.").arg(updateChecker.currentVersion)
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
                text: qsTr("Skip this version")
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
