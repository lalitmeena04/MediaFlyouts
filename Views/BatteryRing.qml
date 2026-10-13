import QtQuick
import MediaFlyouts

Item {
    id: ring
    property real percent: 0
    property color trackColor: "#3A3A3A"
    property color progressColor: settings.batteryRingUseAccent
        ? (settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent)
        : (percent <= 20 ? "#E06C75" : (percent <= 40 ? "#E5C07B" : "#6FCF7F"))
    property color unknownColor: theme.textMuted
    property real thickness: 4
    property bool showGauge: true
    property color outlineColor: theme.textMuted
    property bool charging: false
    // Badge sits on the ring stroke. Its border must match what is behind the ring.
    property color badgeBackground: theme.card
    readonly property bool chargingShown: charging && showGauge
    // Position of the moving highlight, in turns. Driven only while charging.
    property real sweep: 0

    default property alias content: contentItem.children

    NumberAnimation on sweep {
        running: ring.chargingShown && ring.visible
        loops: Animation.Infinite
        from: 0
        to: 1
        duration: 1600
    }
    onChargingShownChanged: {
        if(!chargingShown)
            sweep = 0
        canvas.requestPaint()
    }
    onSweepChanged: canvas.requestPaint()

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

            if(!ring.chargingShown)
                return

            // Comet: short bright head with a fading tail, travelling once per sweep.
            var head = start + Math.PI * 2 * ring.sweep
            var segments = 6
            var segment = Math.PI * 2 * 0.04
            ctx.lineCap = "butt"
            for(var i = 0; i < segments; i++) {
                ctx.beginPath()
                ctx.lineWidth = ring.thickness
                ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.55 * (1 - i / segments))
                ctx.arc(cx, cy, r, head - segment * (i + 1), head - segment * i, false)
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

    // Charging badge. Overlaps the bottom of the ring so it reads as part of the gauge.
    Rectangle {
        id: chargeBadge
        visible: ring.chargingShown
        readonly property int size: Math.max(9, Math.round(ring.thickness * 3))
        width: size
        height: size
        radius: size / 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.bottom
        anchors.verticalCenterOffset: -ring.thickness / 2
        color: ring.percent < 0 ? ring.unknownColor : ring.progressColor
        border.color: ring.badgeBackground
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: Fluent.flash
            font.family: Fluent.fontFamily
            font.pixelSize: Math.round(chargeBadge.size * 0.8)
            color: ring.badgeBackground
        }
    }
}
