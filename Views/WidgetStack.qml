import QtQuick
import MediaFlyouts

QtObject {
    id: stack
    property var mediaWidget: null
    property var peripheralsWidget: null

    readonly property bool enabled: settings.taskbarWidgetLayout === 1

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

    property string wanted: ""
    readonly property string current: pages.indexOf(wanted) !== -1 ? wanted : (pages.length > 0 ? pages[0] : "")
    readonly property bool showsMedia: !enabled || current === mediaPage
    readonly property string activeSection: enabled && current !== mediaPage ? current : ""

    readonly property int frameWidth: Math.max(mediaWidget ? mediaWidget.naturalWidth : 0,
                                               peripheralsWidget ? peripheralsWidget.widestSectionWidth : 0)
    readonly property int frameHeight: Math.max(mediaWidget ? mediaWidget.naturalHeight : 0,
                                                peripheralsWidget ? peripheralsWidget.naturalHeight : 0)

    property int lastStep: 1

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
