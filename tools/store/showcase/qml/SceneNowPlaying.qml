import QtQuick

Poster {
    backdrop.flipped: true

    Part { name: "nowplaying-card"; zoom: 4.4; x: 300; y: 180 }
    Part { name: "nowplaying-pill"; zoom: 4.4; x: 2560 - width - 300; y: 700 }
}
