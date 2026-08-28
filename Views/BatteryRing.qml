import QtQuick
import MediaFlyouts

// A circular battery gauge (PS5-widget style): a dim track ring plus a colored progress
// arc for the percentage, with room for a centered icon via the default 'content' alias.
Item {
    id: ring
    property real percent: 0
    property color trackColor: "#3A3A3A"
    // by default the arc color reflects charge level; when enabled in settings it instead
    // follows the app accent (which itself follows album art when that mode is on)
    property color progressColor: settings.batteryRingUseAccent
        ? (settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent)
        : (percent <= 20 ? "#E06C75" : (percent <= 40 ? "#E5C07B" : "#6FCF7F"))
    property real thickness: 4

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
            ctx.strokeStyle = ring.trackColor
            ctx.arc(cx, cy, r, 0, Math.PI * 2, false)
            ctx.stroke()

            var pct = Math.max(0, Math.min(100, ring.percent)) / 100
            if(pct > 0) {
                ctx.beginPath()
                ctx.lineWidth = ring.thickness
                ctx.strokeStyle = ring.progressColor
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
    Component.onCompleted: canvas.requestPaint()

    Item {
        id: contentItem
        anchors.fill: parent
    }
}
