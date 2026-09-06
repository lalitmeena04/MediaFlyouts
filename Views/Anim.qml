pragma Singleton
import QtQuick

// Shared animation helpers. All flyouts use same easing mapping.
QtObject {
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
