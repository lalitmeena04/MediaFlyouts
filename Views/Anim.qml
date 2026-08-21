pragma Singleton
import QtQuick

// Shared animation helpers so every flyout uses the same easing mapping.
QtObject {
    // maps the flyoutAnimationEasingStyle setting (0..3) to a QML easing curve
    function easingFor(style) {
        switch(style) {
        case 0: return Easing.Linear
        case 1: return Easing.OutSine
        case 2: return Easing.OutQuad
        default: return Easing.OutCubic
        }
    }
}
