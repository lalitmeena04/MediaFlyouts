import QtQuick

// A single Fluent icon glyph rendered in the bundled icon font.
Text {
    property string glyph: ""
    property int size: 16

    text: glyph
    font.family: Fluent.fontFamily
    font.pixelSize: size
    color: "#E0E0E0"
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
}
