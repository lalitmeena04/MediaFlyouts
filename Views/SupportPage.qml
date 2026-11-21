import QtQuick
import QtQuick.Controls.Basic

// Dashboard "Support" page. Shows direct payment details or a web link.
Column {
    id: page
    property var ownerWindow: null
    property string copiedText: ""
    spacing: 12

    function copy(text) {
        donate.copyToClipboard(text)
        page.copiedText = text
        copiedTimer.restart()
    }
    Timer {
        id: copiedTimer
        interval: 1800
        onTriggered: page.copiedText = ""
    }

    Text {
        text: qsTr("Support MediaFlyouts")
        color: theme.textPrimary
        font.pixelSize: 16
        font.bold: true
    }
    Text {
        width: parent.width
        wrapMode: Text.WordWrap
        text: qsTr("MediaFlyouts is built by one developer. If it earns a place on your taskbar, a donation helps keep it going. Donations are voluntary and unlock nothing.")
        color: theme.textSecondary
        font.pixelSize: 13
    }
    Text {
        width: parent.width
        wrapMode: Text.WordWrap
        text: qsTr("Microsoft is not the fundraiser or sponsor of these donations.")
        color: theme.textMuted
        font.pixelSize: 12
    }

    Row {
        spacing: 12
        visible: donate.hasPage
        Button {
            text: qsTr("Open donation page")
            onClicked: donate.openPage()
        }
    }

    Column {
        width: parent.width
        spacing: 12
        visible: donate.showDirect

        Column {
            width: parent.width
            spacing: 8
            visible: donate.hasUpi

            Text {
                text: qsTr("UPI")
                color: theme.textPrimary
                font.pixelSize: 15
                font.bold: true
                topPadding: 8
            }
            Row {
                spacing: 16
                Rectangle {
                    width: 164
                    height: 164
                    radius: 8
                    color: "white"
                    Image {
                        id: upiQr
                        anchors.centerIn: parent
                        width: 148
                        height: 148
                        sourceSize: Qt.size(148, 148)
                        smooth: false
                        source: donate.upiQrSource
                    }
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6
                    Text {
                        text: qsTr("Scan with any UPI app")
                        color: theme.textSecondary
                        font.pixelSize: 13
                    }
                    Text {
                        text: donate.upiId
                        color: theme.textPrimary
                        font.pixelSize: 15
                        font.bold: true
                    }
                    Text {
                        visible: donate.payeeName.length > 0
                        text: donate.payeeName
                        color: theme.textTertiary
                        font.pixelSize: 12
                    }
                    Row {
                        spacing: 8
                        Button {
                            text: qsTr("Copy UPI ID")
                            onClicked: page.copy(donate.upiId)
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            visible: page.copiedText === donate.upiId
                            text: qsTr("Copied")
                            color: theme.accent
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }

        Column {
            width: parent.width
            spacing: 6
            visible: donate.hasBank

            Text {
                text: qsTr("Bank transfer (NEFT / IMPS)")
                color: theme.textPrimary
                font.pixelSize: 15
                font.bold: true
                topPadding: 8
            }
            Repeater {
                model: [
                    { "label": qsTr("Account holder"), "value": donate.bankHolder },
                    { "label": qsTr("Bank"), "value": donate.bankName },
                    { "label": qsTr("Account number"), "value": donate.bankAccount },
                    { "label": qsTr("IFSC"), "value": donate.bankIfsc }
                ]
                delegate: Row {
                    required property var modelData
                    visible: modelData.value.length > 0
                    spacing: 12
                    height: 32
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 130
                        text: modelData.label
                        color: theme.textSecondary
                        font.pixelSize: 13
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 220
                        text: modelData.value
                        color: theme.textPrimary
                        font.pixelSize: 13
                        font.bold: true
                        elide: Text.ElideRight
                    }
                    Button {
                        anchors.verticalCenter: parent.verticalCenter
                        text: page.copiedText === modelData.value ? qsTr("Copied") : qsTr("Copy")
                        onClicked: page.copy(modelData.value)
                    }
                }
            }
        }
    }

}
