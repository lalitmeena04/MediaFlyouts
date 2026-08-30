pragma Singleton
import QtQuick
import MediaFlyouts

// Device-type glyph for the battery widgets. Prefers the real device category (decoded
// from the Bluetooth Class of Device where available); falls back to a name-keyword guess.
// Icons come from Fluent, with Font Awesome filling gaps Fluent lacks (e.g. a mouse).
QtObject {
    // Font Awesome only covers peripherals Fluent has no glyph for; loaded here since this
    // is its sole consumer
    property FontLoader _fa: FontLoader {
        source: "qrc:/qt/qml/MediaFlyouts/assets/fonts/FontAwesome6-Solid.ttf"
    }
    readonly property string faFamily: _fa.name

    // resolves a device to { glyph, fa }; fa marks a Font Awesome glyph so the caller can
    // pick the matching font family
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
            return { "glyph": "\uF8CC", "fa": true } // Font Awesome computer-mouse; Fluent has none
        if(cat === "keyboard")
            return { "glyph": Fluent.keyboard, "fa": false }
        if(cat === "gamepad" || connectionKind === "gamepad")
            return { "glyph": Fluent.gamepad, "fa": false }
        if(cat === "headset")
            return { "glyph": Fluent.headset, "fa": false }
        return { "glyph": Fluent.headphones, "fa": false } // default: headphones / earbuds / other audio
    }

    function glyph(name, connectionKind, category) {
        return _resolve(name, connectionKind, category).glyph
    }

    // font family to render glyph() with, since Fluent and Font Awesome are separate fonts
    function family(name, connectionKind, category) {
        return _resolve(name, connectionKind, category).fa ? faFamily : Fluent.fontFamily
    }

    // How the device is connected, for tooltips.
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
}
