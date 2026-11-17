import QtQuick

// Headline and subtitle at the bottom of a poster.
Item {
    id: caption

    property string headline: ""
    property string subtitle: ""

    // Darkens the backdrop so the text stays readable.
    Rectangle {
        anchors.fill: parent
        anchors.topMargin: -160
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00030510" }
            GradientStop { position: 0.55; color: "#B0030510" }
            GradientStop { position: 1.0; color: "#E0030510" }
        }
    }

    Column {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -6
        width: parent.width - 240
        spacing: 12

        // Long translations shrink to stay on one line.
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: caption.headline
            color: "#FFFFFF"
            font.family: showcase.captionFont
            font.pixelSize: 112
            font.weight: Font.DemiBold
            fontSizeMode: Text.HorizontalFit
            minimumPixelSize: 56
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: caption.subtitle
            color: "#C6D2F5"
            font.family: showcase.captionFont
            font.pixelSize: 54
            fontSizeMode: Text.HorizontalFit
            minimumPixelSize: 34
        }
    }
}
