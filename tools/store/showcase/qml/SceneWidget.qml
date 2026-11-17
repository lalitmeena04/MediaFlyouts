import QtQuick

Poster {
    id: scene
    backdrop.flipped: true

    // Three taskbars, one per visualizer style.
    Item {
        id: group
        width: 3200
        height: 1100
        x: -320
        y: 20
        transform: Matrix4x4 {
            matrix: scene.tilt(-14, 4, -5, 6000, group.width / 2, group.height / 2)
        }

        TaskbarStrip {
            id: first
            zoom: 5.4
            width: parent.width
            y: 90
            tiles: 0
            Part { name: "widget-bars"; zoom: first.zoom; shadow: false; x: 520; anchors.verticalCenter: parent.verticalCenter }
        }
        TaskbarStrip {
            id: second
            zoom: 5.4
            width: parent.width
            y: 420
            tiles: 0
            Part { name: "widget-wave"; zoom: second.zoom; shadow: false; x: 900; anchors.verticalCenter: parent.verticalCenter }
        }
        TaskbarStrip {
            id: third
            zoom: 5.4
            width: parent.width
            y: 750
            tiles: 0
            Part { name: "widget-radial"; zoom: third.zoom; shadow: false; x: 1280; anchors.verticalCenter: parent.verticalCenter }
        }
    }
}
