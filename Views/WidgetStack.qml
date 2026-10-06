import QtQuick
import MediaFlyouts

// Pages media and peripheral widgets through one taskbar slot.
// Active only in stack layout. Pages: media, then one per peripheral section.
QtObject {
    id: stack
    property var mediaWidget: null
    property var peripheralsWidget: null

    readonly property bool enabled: settings.taskbarWidgetLayout === 1

    // Media page key. Peripheral pages use the section label.
    readonly property string mediaPage: "media"

    readonly property var pages: {
        var list = []
        if(!stack.enabled)
            return list
        if(stack.mediaWidget && stack.mediaWidget.eligible)
            list.push(stack.mediaPage)
        if(stack.peripheralsWidget && stack.peripheralsWidget.eligible) {
            var sections = stack.peripheralsWidget.deviceSections
            for(var i = 0; i < sections.length; i++)
                list.push(sections[i].label)
        }
        return list
    }

    // Page the user picked. Falls back to first page when it disappears.
    property string wanted: ""
    readonly property string current: pages.indexOf(wanted) !== -1 ? wanted : (pages.length > 0 ? pages[0] : "")
    readonly property bool showsMedia: !enabled || current === mediaPage
    // Empty means all sections. Set only in stack layout.
    readonly property string activeSection: enabled && current !== mediaPage ? current : ""

    // Every page uses one frame. It fits the largest widget. Content centers inside.
    readonly property int frameWidth: Math.max(mediaWidget ? mediaWidget.naturalWidth : 0,
                                               peripheralsWidget ? peripheralsWidget.widestSectionWidth : 0)
    readonly property int frameHeight: Math.max(mediaWidget ? mediaWidget.naturalHeight : 0,
                                                peripheralsWidget ? peripheralsWidget.naturalHeight : 0)

    // Direction of the last page change. Transitions slide this way.
    property int lastStep: 1

    // Throttle paging from touchpad inertia.
    property bool cooldown: false
    property Timer cooldownTimer: Timer {
        interval: 280
        onTriggered: stack.cooldown = false
    }

    function cycle(step) {
        if(pages.length < 2 || stack.cooldown)
            return
        var i = pages.indexOf(current)
        lastStep = step > 0 ? 1 : -1
        wanted = pages[(i + step + pages.length) % pages.length]
        stack.cooldown = true
        cooldownTimer.restart()
    }
}
