import QtQuick

Poster {
    id: scene
    backdrop.flipped: true

    // A wall of flyouts, tilted away from the viewer.
    Item {
        id: wall
        width: 3600
        height: 1700
        x: -520
        y: -330
        transform: Matrix4x4 {
            matrix: scene.tilt(-20, 9, -7, 5200, wall.width / 2, wall.height / 2)
        }

        Part { name: "nowplaying-pill"; zoom: 2.8; x: 420; y: 330 }
        Part { name: "media-a"; zoom: 2.8; x: 300; y: 580 }
        Part { name: "peripheral-flyout"; zoom: 2.8; x: 120; y: 1070 }

        Part { name: "media-b"; zoom: 2.8; x: 1440; y: 300 }
        Part { name: "volume"; zoom: 2.8; x: 1520; y: 760 }
        Part { name: "lock-caps"; zoom: 2.8; x: 1720; y: 1000 }
        Part { name: "media-compact"; zoom: 2.8; x: 1700; y: 1280 }

        Part { name: "volume-mixer"; zoom: 2.8; x: 2580; y: 60 }
        Part { name: "media-c"; zoom: 2.8; x: 2480; y: 1010 }
        Part { name: "lock-num"; zoom: 2.8; x: 2700; y: 1480 }
    }
}
