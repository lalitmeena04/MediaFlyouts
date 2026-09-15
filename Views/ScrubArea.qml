import QtQuick

// Drag gesture for horizontal tracks. Reports the pointer as a 0..1 fraction.
MouseArea {
    id: scrub

    // Width that maps pixels to a fraction. Defaults to the track it covers.
    property real trackWidth: parent ? parent.width : 0

    signal scrubStarted(real fraction)
    signal scrubMoved(real fraction)
    signal scrubEnded(real fraction)

    function fractionAt(px) {
        if(scrub.trackWidth <= 0)
            return 0
        return Math.max(0, Math.min(1, px / scrub.trackWidth))
    }

    anchors.fill: parent

    onPressed: (mouse) => scrub.scrubStarted(scrub.fractionAt(mouse.x))
    onPositionChanged: (mouse) => {
        if(scrub.pressed)
            scrub.scrubMoved(scrub.fractionAt(mouse.x))
    }
    onReleased: (mouse) => scrub.scrubEnded(scrub.fractionAt(mouse.x))
}
