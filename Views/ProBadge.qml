import QtQuick

Rectangle {
    id: badge
    radius: 4
    color: theme.accentBadge
    height: label.implicitHeight + 4
    width: label.implicitWidth + 10

    Text {
        id: label
        anchors.centerIn: parent
        text: qsTr("PRO")
        color: theme.accentSoft
        font.pixelSize: 9
        font.bold: true
        font.letterSpacing: 1
    }
}
