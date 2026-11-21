pragma Singleton
import QtQuick

QtObject {
    readonly property int moveMs: settings.reduceMotion ? 0 : 150
    readonly property int fillMs: settings.reduceMotion ? 0 : 250
    readonly property int hoverMs: settings.reduceMotion ? 0 : 100

    function easingFor(style) {
        switch(style) {
        case 0: return Easing.Linear
        case 1: return Easing.OutSine
        case 2: return Easing.OutQuad
        default: return Easing.OutCubic
        }
    }
}
