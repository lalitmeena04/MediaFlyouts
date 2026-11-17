import QtQuick

// A generic taskbar strip to dock the widgets on. Deliberately neutral: no
// operating system logo and no third-party app icons.
Item {
    id: strip

    property real zoom: 4
    // Number of placeholder app tiles.
    property int tiles: 5
    // Where the tile row is centred, as a fraction of the width.
    property real tilesCenter: 0.5
    property string clock: "10:24"
    property string date: "16 Nov"
    default property alias content: slot.data

    height: 48 * strip.zoom

    Rectangle {
        anchors.fill: parent
        color: "#F214161C"
    }
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: Math.max(2, strip.zoom * 0.5)
        color: "#26FFFFFF"
    }

    Row {
        x: strip.width * strip.tilesCenter - width / 2
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12 * strip.zoom
        Repeater {
            model: strip.tiles
            delegate: Item {
                required property int index
                width: 26 * strip.zoom
                height: 34 * strip.zoom
                Rectangle {
                    width: parent.width
                    height: parent.width
                    radius: 6 * strip.zoom
                    color: index === 0 ? "#3CFFFFFF" : "#24FFFFFF"
                }
                Rectangle {
                    visible: index === 2 || index === 3
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    width: (index === 2 ? 12 : 5) * strip.zoom
                    height: 3 * strip.zoom
                    radius: height / 2
                    color: index === 2 ? "#B0FFFFFF" : "#60FFFFFF"
                }
            }
        }
    }

    Column {
        anchors.right: parent.right
        anchors.rightMargin: 18 * strip.zoom
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1 * strip.zoom
        Text {
            anchors.right: parent.right
            text: strip.clock
            color: "#E6FFFFFF"
            font.family: "Segoe UI"
            font.pixelSize: 12 * strip.zoom
        }
        Text {
            anchors.right: parent.right
            text: strip.date
            color: "#E6FFFFFF"
            font.family: "Segoe UI"
            font.pixelSize: 12 * strip.zoom
        }
    }

    Item {
        id: slot
        anchors.fill: parent
    }
}
