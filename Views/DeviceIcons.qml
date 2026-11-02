pragma Singleton
import QtQuick
import MediaFlyouts

QtObject {
    property FontLoader _fa: FontLoader {
        source: "qrc:/qt/qml/MediaFlyouts/assets/fonts/FontAwesome6-Solid.ttf"
    }
    readonly property string faFamily: _fa.name

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
        return { "glyph": Fluent.headphones, "fa": false }
    }

    function glyph(name, connectionKind, category) {
        return _resolve(name, connectionKind, category).glyph
    }

    function family(name, connectionKind, category) {
        return _resolve(name, connectionKind, category).fa ? faFamily : Fluent.fontFamily
    }

    function transport(connectionKind) {
        switch(connectionKind) {
        case "bluetooth":
        case "ble":
            return qsTr("Bluetooth")
        case "wireless":
            return qsTr("2.4 GHz")
        case "usb":
            return qsTr("USB")
        }
        return ""
    }

    // A wired device normally has no battery. One that reports a level does.
    function hasBattery(connectionKind, percent) {
        return connectionKind !== "usb" || percent >= 0
    }
}
