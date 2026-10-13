import QtQuick

IconLabel {
    id: button
    property string tooltip: ""
    property var tip: null
    property color idleColor: "#C8C8C8"
    property color hoverColor: "white"
    signal clicked()

    color: area.containsMouse ? button.hoverColor : button.idleColor

    function refreshTip() {
        if(!button.tip)
            return
        if(area.containsMouse && button.tooltip.length > 0)
            button.tip.showAt(button.mapToGlobal(button.width / 2, 0), button.tooltip)
        else
            button.tip.hide()
    }
    onTooltipChanged: refreshTip()

    MouseArea {
        id: area
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: button.clicked()
        onContainsMouseChanged: button.refreshTip()
    }
}
