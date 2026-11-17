import QtQuick

Poster {
    Part {
        name: "media-a"
        zoom: 3.7
        x: (2560 - width) / 2
        y: 150
    }

    TaskbarStrip {
        id: bar
        zoom: 3.7
        x: 0
        width: 2560
        y: 800
        tiles: 4
        tilesCenter: 0.64

        Part {
            name: "widget-bars"
            zoom: bar.zoom
            shadow: false
            x: 16 * bar.zoom
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
