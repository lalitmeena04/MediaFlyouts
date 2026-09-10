import QtQuick
import MediaFlyouts

// Circular battery gauge. Shows track ring and progress arc.
// Default content alias holds centered icon.
Item {
    id: ring
    property real percent: 0
    property color trackColor: "#3A3A3A"
    // Arc color reflects charge level by default.
    // Setting can make it follow app accent.
    property color progressColor: settings.batteryRingUseAccent
        ? (settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent)
        : (percent <= 20 ? "#E06C75" : (percent <= 40 ? "#E5C07B" : "#6FCF7F"))
    property color unknownColor: theme.textMuted
    property real thickness: 4
    // False keeps the outline but drops the charge arc, for devices with no battery.
    property bool showGauge: true
    // Outline for a device with no battery. Solid stroke, nothing filled in.
    property color outlineColor: theme.textMuted

    default property alias content: contentItem.children

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var cx = width / 2
            var cy = height / 2
            var r = Math.min(width, height) / 2 - ring.thickness / 2
            var start = -Math.PI / 2

            ctx.beginPath()
            ctx.lineWidth = ring.thickness
            ctx.strokeStyle = ring.showGauge ? ring.trackColor : ring.outlineColor
            ctx.arc(cx, cy, r, 0, Math.PI * 2, false)
            ctx.stroke()

            if(!ring.showGauge)
                return

            // Unknown battery draws full neutral ring. Device reads as connected.
            var unknown = ring.percent < 0
            var pct = unknown ? 1 : Math.max(0, Math.min(100, ring.percent)) / 100
            if(pct > 0) {
                ctx.beginPath()
                ctx.lineWidth = ring.thickness
                ctx.strokeStyle = unknown ? ring.unknownColor : ring.progressColor
                ctx.lineCap = "round"
                ctx.arc(cx, cy, r, start, start + Math.PI * 2 * pct, false)
                ctx.stroke()
            }
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    onPercentChanged: canvas.requestPaint()
    onProgressColorChanged: canvas.requestPaint()
    onUnknownColorChanged: canvas.requestPaint()
    onShowGaugeChanged: canvas.requestPaint()
    onOutlineColorChanged: canvas.requestPaint()
    Component.onCompleted: canvas.requestPaint()

    Item {
        id: contentItem
        anchors.fill: parent
    }
}
