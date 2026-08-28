pragma Singleton
import QtQuick

// Best-effort device-type glyph for the battery widgets. Prefers the real device
// category (decoded from the Bluetooth Class of Device where available); falls back to
// a name-keyword guess, and finally to the connection kind, when category is unknown.
QtObject {
    function glyph(name, connectionKind, category) {
        if(category === "mouse")
            return "\uD83D\uDDB1" // 🖱
        if(category === "keyboard")
            return "\u2328" // ⌨
        if(category === "headset")
            return "\uD83C\uDFA7" // 🎧
        if(category === "gamepad" || connectionKind === "gamepad")
            return "\uD83C\uDFAE" // 🎮

        var n = (name || "").toLowerCase()
        if(n.indexOf("mouse") >= 0)
            return "\uD83D\uDDB1"
        if(n.indexOf("keyboard") >= 0 || n.indexOf(" kb") >= 0)
            return "\u2328"
        return "\uD83C\uDFA7" // default: headset / earbuds / other audio
    }
}
