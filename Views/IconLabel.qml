import QtQuick

// A single Fluent icon glyph rendered in the bundled icon font.
Text {
    property string glyph: ""
    property int size: 16

    text: glyph
    font.family: Fluent.fontFamily
    font.pixelSize: size
    color: theme.textPrimary
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
}
