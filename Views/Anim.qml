pragma Singleton
import QtQuick

// Shared animation helpers. All flyouts use same easing mapping.
QtObject {
    // Window or widget moves to a new place.
    readonly property int moveMs: 150
    // Progress or level fills follow a new value.
    readonly property int fillMs: 250
    // Control reacts to hover or press.
    readonly property int hoverMs: 100

    // Map flyoutAnimationEasingStyle setting to QML easing curve.
    function easingFor(style) {
        switch(style) {
        case 0: return Easing.Linear
        case 1: return Easing.OutSine
        case 2: return Easing.OutQuad
        default: return Easing.OutCubic
        }
    }
}
