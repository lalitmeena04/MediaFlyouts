import QtQuick

Poster {
    backdrop.flipped: true

    Part { name: "lock-num"; zoom: 3.5; x: 170; y: 150 }
    Part { name: "lock-caps"; zoom: 4.8; x: (2560 - width) / 2; y: 380 }
    Part { name: "lock-scroll"; zoom: 3.5; x: 2560 - width - 170; y: 730 }
}
