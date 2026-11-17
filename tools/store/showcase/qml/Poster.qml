import QtQuick
import QtQuick.Window

// One Store screenshot. Scenes lay out their content in a fixed 2560x1440
// design space; the stage scales it to the requested output size.
Window {
    id: poster
    width: showcase.posterWidth
    height: showcase.posterHeight
    visible: false
    color: "#05060F"

    readonly property int designWidth: 2560
    readonly property int designHeight: 1440
    // Space the caption takes at the bottom, in design units.
    readonly property int captionHeight: poster.headline.length > 0 ? 330 : 0

    default property alias content: stage.data
    property alias backdrop: backdrop
    // Poster name. Selects the texts in captions.json.
    property string name: ""
    readonly property var texts: poster.name.length > 0 ? showcase.caption(poster.name) : ({})
    readonly property string headline: poster.texts.headline || ""
    readonly property string subtitle: poster.texts.subtitle || ""

    // Matrix for a plane tilted in 3D around (cx, cy). Angles in degrees.
    // A larger distance gives a weaker perspective.
    function tilt(yaw, pitch, roll, distance, cx, cy) {
        var toRad = Math.PI / 180
        var cy_ = Math.cos(yaw * toRad), sy = Math.sin(yaw * toRad)
        var cp = Math.cos(pitch * toRad), sp = Math.sin(pitch * toRad)
        var cr = Math.cos(roll * toRad), sr = Math.sin(roll * toRad)
        var rotY = Qt.matrix4x4(cy_, 0, sy, 0,  0, 1, 0, 0,  -sy, 0, cy_, 0,  0, 0, 0, 1)
        var rotX = Qt.matrix4x4(1, 0, 0, 0,  0, cp, -sp, 0,  0, sp, cp, 0,  0, 0, 0, 1)
        var rotZ = Qt.matrix4x4(cr, -sr, 0, 0,  sr, cr, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1)
        var r = rotZ.times(rotX).times(rotY)
        // Flatten depth into the perspective divide. Qt Quick has no depth axis.
        var depth = r.row(2)
        var projected = Qt.matrix4x4(r.m11, r.m12, r.m13, r.m14,
                                     r.m21, r.m22, r.m23, r.m24,
                                     0, 0, 1, 0,
                                     -depth.x / distance, -depth.y / distance, -depth.z / distance, 1)
        var there = Qt.matrix4x4(1, 0, 0, cx,  0, 1, 0, cy,  0, 0, 1, 0,  0, 0, 0, 1)
        var back = Qt.matrix4x4(1, 0, 0, -cx,  0, 1, 0, -cy,  0, 0, 1, 0,  0, 0, 0, 1)
        return there.times(projected).times(back)
    }

    Item {
        width: poster.designWidth
        height: poster.designHeight
        scale: poster.width / poster.designWidth
        transformOrigin: Item.TopLeft
        clip: true

        Backdrop {
            id: backdrop
            anchors.fill: parent
        }

        Item {
            id: stage
            anchors.fill: parent
        }

        Caption {
            visible: poster.headline.length > 0
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: poster.captionHeight
            headline: poster.headline
            subtitle: poster.subtitle
        }
    }
}
