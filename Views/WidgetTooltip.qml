import QtQuick
import QtQuick.Window

// Popup or ToolTip would clip to the widget window bounds.
Window {
    id: tip
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus
    color: "transparent"
    width: tipText.implicitWidth + 16
    height: tipText.implicitHeight + 10

    property int monitor: settings.taskbarWidgetMonitor

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: "#333333"
        border.color: "#4A4A4A"

        Text {
            id: tipText
            anchors.centerIn: parent
            color: "#EEEEEE"
            font.pixelSize: 11
        }
    }

    function showAt(globalPoint, text) {
        tipText.text = text
        var mon = monitors.geometry(tip.monitor)
        var x = globalPoint.x - tip.width / 2
        if(mon.width > 0)
            x = Math.max(mon.x + 4, Math.min(x, mon.x + mon.width - tip.width - 4))
        tip.x = x
        tip.y = globalPoint.y - tip.height - 6
        tip.visible = true
    }
    function hide() {
        tip.visible = false
    }
}
