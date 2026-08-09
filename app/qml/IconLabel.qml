import QtQuick

// A single Fluent icon glyph rendered in the bundled icon font.
Text {
    property string glyph: ""
    property int size: 16

    text: glyph
    font.family: iconFontFamily
    font.pixelSize: size
    color: "#E0E0E0"
    renderType: Text.NativeRendering
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
}
