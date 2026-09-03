pragma Singleton

import QtQuick

// Fluent System Icons (Filled) glyph codepoints.
QtObject {
    property FontLoader _loader: FontLoader {
        source: "qrc:/qt/qml/MediaFlyouts/assets/fonts/FluentSystemIcons-Filled.ttf"
    }
    readonly property string fontFamily: _loader.name

    readonly property string previous: "\uF633"
    readonly property string next: "\uF574"
    readonly property string play: "\uF610"
    readonly property string pause: "\uF5AC"
    readonly property string speaker2: "\uEB4C"
    readonly property string speaker1: "\uF704"
    readonly property string speaker0: "\uF702"
    readonly property string speakerMute: "\uEB54"
    readonly property string musicNote: "\uE863"
    readonly property string settings: "\uF6B3"
    readonly property string dismiss: "\uF36A"
    readonly property string chevronDoubleLeft: "\uF1A7"
    readonly property string chevronDoubleRight: "\uF1A8"
    readonly property string keyboard: "\uF4C3"
    readonly property string headphones: "\uF476"
    readonly property string headset: "\uF478"
    readonly property string gamepad: "\uEEB9"
    readonly property string bluetooth: "\uE15E"
    readonly property string usbStick: "\uF857"
    readonly property string repeatAll: "\uF172"
    readonly property string repeatOne: "\uEF34"
    readonly property string shuffle: "\uEF37"
    readonly property string info: "\uF4A4"
    readonly property string link: "\uF4E9"
    readonly property string battery: "\uE14E"
    readonly property string chevronLeft: "\uEA4D"
    readonly property string chevronRight: "\uEA50"
}
