import QtQuick
import MediaFlyouts

// Non-visual app root: owns the flyout + dashboard windows and reacts to tray intents.
QtObject {
    id: app

    property MediaFlyout flyout: MediaFlyout {
        onVisibleChanged: app.restack()
    }
    property VolumeFlyout volumeFlyout: VolumeFlyout {
        onVisibleChanged: app.restack()
    }
    property LockKeysFlyout lockKeysFlyout: LockKeysFlyout {
        onVisibleChanged: app.restack()
    }
    property NextUpFlyout nextUpFlyout: NextUpFlyout {
        onVisibleChanged: app.restack()
    }
    property Dashboard dashboard: Dashboard {}
    property TaskbarWidget taskbarWidget: TaskbarWidget {
        onActivated: flyout.showFlyout()
    }

    // stacking order, bottom-most first; visible flyouts are stacked away from the anchor edge
    readonly property var flyoutStack: [flyout, volumeFlyout, lockKeysFlyout, nextUpFlyout]

    function restack() {
        var topAligned = settings.flyoutPosition <= 2 // 0..2 are the top positions
        var gap = 10
        var cumulative = 0
        for(var i = 0; i < flyoutStack.length; i++) {
            var f = flyoutStack[i]
            if(!f.visible) {
                f.positioned = false // next appearance snaps in without sliding
                continue
            }
            var p = placement.position(f.width, f.height)
            f.x = p.x
            f.y = topAligned ? (p.y + cumulative) : (p.y - cumulative)
            cumulative += f.height + gap
            f.positioned = true // subsequent shifts animate
        }
    }

    // re-stack when the position / monitor settings change
    property Connections settingsConnection: Connections {
        target: settings
        function onChanged() { app.restack() }
    }

    property Connections trayConnection: Connections {
        target: tray

        function onOpenDashboardRequested() {
            dashboard.show()
            dashboard.raise()
            dashboard.requestActivate()
        }

        function onShowFlyoutRequested() {
            flyout.showFlyout()
        }
    }

    // a second launch asks us to surface the dashboard
    property Connections instanceConnection: Connections {
        target: instance
        function onActivated() {
            dashboard.show()
            dashboard.raise()
            dashboard.requestActivate()
        }
    }
}
