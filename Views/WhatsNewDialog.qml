import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import MediaFlyouts

Window {
    id: root
    width: 520
    height: 420
    minimumWidth: 440
    minimumHeight: 320
    title: qsTr("What's new")
    color: theme.window
    flags: Qt.Dialog
    visible: false

    Component.onCompleted: windowEffects.setDarkFrame(root, theme.dark)

    function open() {
        root.show()
        root.raise()
        root.requestActivate()
    }

    onClosing: whatsNew.markSeen()

    Item {
        anchors.fill: parent
        anchors.margins: 24

        Row {
            id: header
            anchors.top: parent.top
            anchors.left: parent.left
            spacing: 12
            AppLogo {
                anchors.verticalCenter: parent.verticalCenter
                size: 28
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("What's new in MediaFlyouts %1").arg(whatsNew.version)
                color: theme.textPrimary
                font.pixelSize: 17
                font.bold: true
            }
        }

        Flickable {
            id: notes
            anchors.top: header.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttons.top
            anchors.bottomMargin: 16
            clip: true
            contentWidth: width
            contentHeight: body.implicitHeight
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: notes.contentHeight > notes.height ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff }

            Text {
                id: body
                width: notes.width - 12
                text: whatsNew.body
                textFormat: Text.MarkdownText
                wrapMode: Text.WordWrap
                color: theme.textSecondary
                font.pixelSize: 13
                lineHeight: 1.25
            }
        }

        Row {
            id: buttons
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            spacing: 8
            Button {
                text: qsTr("Full release notes")
                onClicked: updateChecker.openReleasePage()
            }
            Button {
                text: qsTr("Close")
                onClicked: root.close()
            }
        }
    }
}
