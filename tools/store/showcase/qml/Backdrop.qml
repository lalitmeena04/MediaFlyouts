import QtQuick
import QtQuick.Shapes

// Abstract backdrop: a dark gradient, soft glows and layered ribbons.
// Drawn here so the screenshots carry no third-party wallpaper.
Item {
    id: backdrop

    property color deep: "#050816"
    property color mid: "#0D1A4A"
    property color glowA: "#2F7BFF"
    property color glowB: "#7B3FF2"
    property color glowC: "#19C6D6"
    // Mirrors the ribbons, so neighbouring posters differ.
    property bool flipped: false

    function tint(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: backdrop.deep }
            GradientStop { position: 1.0; color: backdrop.mid }
        }
    }

    component Glow: Shape {
        id: glow
        property real cx: 0
        property real cy: 0
        property real radius: 100
        property color tint: "white"
        property real strength: 0.5
        anchors.fill: parent
        ShapePath {
            strokeColor: "transparent"
            fillGradient: RadialGradient {
                centerX: glow.cx
                centerY: glow.cy
                centerRadius: glow.radius
                focalX: glow.cx
                focalY: glow.cy
                GradientStop { position: 0.0; color: backdrop.tint(glow.tint, glow.strength) }
                GradientStop { position: 1.0; color: backdrop.tint(glow.tint, 0) }
            }
            startX: 0
            startY: 0
            PathLine { x: glow.width; y: 0 }
            PathLine { x: glow.width; y: glow.height }
            PathLine { x: 0; y: glow.height }
            PathLine { x: 0; y: 0 }
        }
    }

    // A band whose crest runs left to right. Everything below the crest is filled.
    component Ribbon: Shape {
        id: ribbon
        property real leftY: 0
        property real rightY: 0
        property real c1x: 0
        property real c1y: 0
        property real c2x: 0
        property real c2y: 0
        property color crest: "white"
        property color foot: "black"
        property real strength: 0.6
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer
        ShapePath {
            strokeColor: backdrop.tint(Qt.lighter(ribbon.crest, 1.5), 0.35)
            strokeWidth: 3
            fillGradient: LinearGradient {
                x1: ribbon.width * 0.3
                y1: Math.min(ribbon.leftY, ribbon.rightY) - 100
                x2: ribbon.width * 0.6
                y2: ribbon.height
                GradientStop { position: 0.0; color: backdrop.tint(ribbon.crest, ribbon.strength) }
                GradientStop { position: 1.0; color: backdrop.tint(ribbon.foot, ribbon.strength * 0.9) }
            }
            startX: -200
            startY: ribbon.leftY
            PathCubic {
                control1X: ribbon.c1x
                control1Y: ribbon.c1y
                control2X: ribbon.c2x
                control2Y: ribbon.c2y
                x: ribbon.width + 200
                y: ribbon.rightY
            }
            PathLine { x: ribbon.width + 200; y: ribbon.height + 200 }
            PathLine { x: -200; y: ribbon.height + 200 }
            PathLine { x: -200; y: ribbon.leftY }
        }
    }

    Glow { cx: backdrop.width * 0.80; cy: backdrop.height * 0.10; radius: backdrop.width * 0.50; tint: backdrop.glowA; strength: 0.55 }
    Glow { cx: backdrop.width * 0.08; cy: backdrop.height * 0.30; radius: backdrop.width * 0.38; tint: backdrop.glowB; strength: 0.40 }

    Item {
        anchors.fill: parent
        transform: Scale {
            origin.x: backdrop.width / 2
            xScale: backdrop.flipped ? -1 : 1
        }

        Ribbon {
            leftY: backdrop.height * 0.74; rightY: backdrop.height * 0.30
            c1x: backdrop.width * 0.30; c1y: backdrop.height * 0.42
            c2x: backdrop.width * 0.62; c2y: backdrop.height * 0.86
            crest: backdrop.glowA; foot: backdrop.mid; strength: 0.50
        }
        Ribbon {
            leftY: backdrop.height * 0.52; rightY: backdrop.height * 0.62
            c1x: backdrop.width * 0.34; c1y: backdrop.height * 1.02
            c2x: backdrop.width * 0.70; c2y: backdrop.height * 0.36
            crest: backdrop.glowB; foot: backdrop.deep; strength: 0.55
        }
        Ribbon {
            leftY: backdrop.height * 0.94; rightY: backdrop.height * 0.80
            c1x: backdrop.width * 0.28; c1y: backdrop.height * 0.66
            c2x: backdrop.width * 0.66; c2y: backdrop.height * 1.10
            crest: backdrop.glowC; foot: backdrop.deep; strength: 0.45
        }
    }

    Glow { cx: backdrop.width * 0.50; cy: backdrop.height * 1.05; radius: backdrop.width * 0.45; tint: backdrop.deep; strength: 0.85 }
}
