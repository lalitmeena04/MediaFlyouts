import QtQuick
import MediaFlyouts

// Audio visualizer for the taskbar widget. Reuses audioCapture.bands for the
// bars/radial/waveform styles and audioCapture.beat for the particle burst.
Item {
    id: viz
    clip: true

    readonly property int style: settings.visualizerStyle
    readonly property color col: settings.accentFromArt && nowPlaying.hasArt ? nowPlaying.artAccent : theme.accent
    property var particles: []

    // style 0: mirrored / bottom-anchored bars
    Row {
        anchors.fill: parent
        spacing: 2
        visible: viz.style === 0
        Repeater {
            model: audioCapture.bands
            delegate: Item {
                required property var modelData
                width: (viz.width - (audioCapture.bands.length - 1) * 2) / Math.max(1, audioCapture.bands.length)
                height: viz.height
                Rectangle {
                    width: parent.width
                    radius: width / 2
                    opacity: 0.7
                    color: viz.col
                    height: settings.taskbarVisualizerCentered
                        ? Math.max(width, parent.height * modelData)
                        : Math.max(2, parent.height * modelData)
                    y: settings.taskbarVisualizerCentered
                        ? (parent.height - height) / 2
                        : parent.height - height
                    Behavior on height { NumberAnimation { duration: 90 } }
                }
            }
        }
    }

    // styles 1-3: radial rays, mirrored waveform, and beat-driven particles
    Canvas {
        id: field
        anchors.fill: parent
        visible: viz.style !== 0
        onPaint: viz.draw()
    }

    function draw() {
        var ctx = field.getContext("2d")
        var w = field.width
        var h = field.height
        ctx.clearRect(0, 0, w, h)
        var bands = audioCapture.bands
        var n = bands.length
        ctx.lineCap = "round"

        if(viz.style === 1 && n > 0) {
            var cx = w / 2
            var cy = h / 2
            var base = Math.min(w, h) * 0.16
            var maxLen = Math.min(w, h) * 0.5 - base
            ctx.lineWidth = Math.max(1.5, (2 * Math.PI * base) / n * 0.55)
            ctx.strokeStyle = viz.col
            for(var i = 0; i < n; i++) {
                var a = (i / n) * 2 * Math.PI - Math.PI / 2
                var len = base + maxLen * bands[i]
                ctx.globalAlpha = 0.45 + 0.55 * bands[i]
                ctx.beginPath()
                ctx.moveTo(cx + Math.cos(a) * base, cy + Math.sin(a) * base)
                ctx.lineTo(cx + Math.cos(a) * len, cy + Math.sin(a) * len)
                ctx.stroke()
            }
            ctx.globalAlpha = 1
        } else if(viz.style === 2 && n > 1) {
            var mid = h / 2
            var step = w / (n - 1)
            ctx.strokeStyle = viz.col
            ctx.lineWidth = 2
            ctx.lineJoin = "round"
            ctx.globalAlpha = 0.9
            ctx.beginPath()
            for(var j = 0; j < n; j++) {
                var y = mid - bands[j] * mid * 0.9
                if(j === 0) ctx.moveTo(0, y); else ctx.lineTo(j * step, y)
            }
            ctx.stroke()
            ctx.globalAlpha = 0.45
            ctx.beginPath()
            for(var k = 0; k < n; k++) {
                var y2 = mid + bands[k] * mid * 0.9
                if(k === 0) ctx.moveTo(0, y2); else ctx.lineTo(k * step, y2)
            }
            ctx.stroke()
            ctx.globalAlpha = 1
        } else if(viz.style === 3) {
            ctx.fillStyle = viz.col
            for(var p = 0; p < viz.particles.length; p++) {
                var pt = viz.particles[p]
                ctx.globalAlpha = Math.max(0, pt.life)
                ctx.beginPath()
                ctx.arc(pt.x, pt.y, pt.r, 0, 2 * Math.PI)
                ctx.fill()
            }
            ctx.globalAlpha = 1
        }
    }

    function frame() {
        if(viz.style === 3) {
            var arr = viz.particles
            for(var i = arr.length - 1; i >= 0; i--) {
                var p = arr[i]
                p.x += p.vx
                p.y += p.vy
                p.vy += 0.05
                p.life -= 0.02
                if(p.life <= 0) arr.splice(i, 1)
            }
        }
        field.requestPaint()
    }

    function spawn() {
        var cx = field.width / 2
        var cy = field.height / 2
        var count = 6 + Math.floor(audioCapture.level * 20)
        for(var i = 0; i < count; i++) {
            var ang = Math.random() * 2 * Math.PI
            var spd = 1 + Math.random() * 3 * (0.5 + audioCapture.level)
            viz.particles.push({
                x: cx, y: cy,
                vx: Math.cos(ang) * spd,
                vy: Math.sin(ang) * spd - 1,
                r: 1.5 + Math.random() * 2.5,
                life: 1.0
            })
        }
    }

    Timer {
        running: viz.visible && viz.style !== 0
        interval: 33
        repeat: true
        onTriggered: viz.frame()
    }

    Connections {
        target: audioCapture
        enabled: viz.style === 3
        function onBeat() { viz.spawn() }
    }
}
