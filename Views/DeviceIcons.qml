pragma Singleton
import QtQuick

// Best-effort device-type glyph for the battery widgets. We only know a device's
// connection kind ("bluetooth"/"ble"/"gamepad"), not its exact category, so this
// falls back to a name-keyword guess for mice/keyboards and otherwise assumes audio.
QtObject {
    function glyph(name, connectionKind) {
        if(connectionKind === "gamepad")
            return "\uD83C\uDFAE" // 🎮
        var n = (name || "").toLowerCase()
        if(n.indexOf("mouse") >= 0)
            return "\uD83D\uDDB1" // 🖱
        if(n.indexOf("keyboard") >= 0 || n.indexOf(" kb") >= 0)
            return "\u2328" // ⌨
        return "\uD83C\uDFA7" // 🎧 (default: headset / earbuds / other audio)
    }
}
