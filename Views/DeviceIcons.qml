pragma Singleton
import QtQuick
import MediaFlyouts

// Device-type glyph for battery widgets.
// Prefer real device category. Fall back to name-keyword guess.
// Font Awesome fills Fluent gaps.
QtObject {
    // Font Awesome is only used here.
    property FontLoader _fa: FontLoader {
        source: "qrc:/qt/qml/MediaFlyouts/assets/fonts/FontAwesome6-Solid.ttf"
    }
    readonly property string faFamily: _fa.name

    // Resolve device to glyph and font source.
    function _resolve(name, connectionKind, category) {
        var cat = category
        if(!cat) {
            var n = (name || "").toLowerCase()
            if(n.indexOf("mouse") >= 0)
                cat = "mouse"
            else if(n.indexOf("keyboard") >= 0 || n.indexOf(" kb") >= 0)
                cat = "keyboard"
        }
        if(cat === "mouse")
            return { "glyph": "\uF8CC", "fa": true } // Font Awesome computer-mouse. Fluent has none.
        if(cat === "keyboard")
            return { "glyph": Fluent.keyboard, "fa": false }
        if(cat === "camera")
            return { "glyph": Fluent.camera, "fa": false }
        if(cat === "gamepad" || connectionKind === "gamepad")
            return { "glyph": Fluent.gamepad, "fa": false }
        if(cat === "headset")
            return { "glyph": Fluent.headset, "fa": false }
        return { "glyph": Fluent.headphones, "fa": false } // Default audio glyph.
    }

    function glyph(name, connectionKind, category) {
        return _resolve(name, connectionKind, category).glyph
    }

    // Font family for glyph(). Fluent and Font Awesome are separate.
    function family(name, connectionKind, category) {
        return _resolve(name, connectionKind, category).fa ? faFamily : Fluent.fontFamily
    }

    // Device transport label for tooltips.
    function transport(connectionKind) {
        switch(connectionKind) {
        case "bluetooth":
        case "ble":
            return "Bluetooth"
        case "wireless":
            return "2.4 GHz"
        case "usb":
            return "USB"
        }
        return ""
    }

    // Wired devices run off the cable. Absent is not the same as unknown.
    function hasBattery(connectionKind) {
        return connectionKind !== "usb"
    }
}
