import QtQuick

Poster {
    Part {
        name: "peripheral-flyout"
        zoom: 3.8
        x: (2560 - width) / 2
        y: 240
    }

    TaskbarStrip {
        id: bar
        zoom: 4.4
        x: 0
        width: 2560
        y: 770
        tiles: 0

        Part {
            id: media
            name: "widget-plain"
            zoom: bar.zoom
            shadow: false
            x: 16 * bar.zoom
            anchors.verticalCenter: parent.verticalCenter
        }
        Part {
            name: "peripherals-widget"
            zoom: bar.zoom
            shadow: false
            x: media.x + media.width + 8 * bar.zoom
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
