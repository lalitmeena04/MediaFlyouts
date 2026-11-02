import QtQuick
import MediaFlyouts

QtObject {
    id: app

    property MediaFlyout flyout: MediaFlyout {
        onVisibleChanged: app.restack()
        onHeightChanged: app.restack()
        onWidthChanged: app.restack()
    }
    property VolumeFlyout volumeFlyout: VolumeFlyout {
        onVisibleChanged: app.restack()
        onHeightChanged: app.restack()
        onWidthChanged: app.restack()
    }
    property LockKeysFlyout lockKeysFlyout: LockKeysFlyout {
        onVisibleChanged: app.restack()
        onHeightChanged: app.restack()
        onWidthChanged: app.restack()
    }
    property NowPlayingFlyout nextUpFlyout: NowPlayingFlyout {
        onVisibleChanged: app.restack()
        onHeightChanged: app.restack()
        onWidthChanged: app.restack()
    }
    property PeripheralFlyout peripheralFlyout: PeripheralFlyout {
        onVisibleChanged: app.restack()
        onHeightChanged: app.restack()
        onWidthChanged: app.restack()
    }
    property ChargerFlyout chargerFlyout: ChargerFlyout {
        onVisibleChanged: app.restack()
        onHeightChanged: app.restack()
        onWidthChanged: app.restack()
    }
    property Dashboard dashboard: Dashboard {}
    property VolumeMixerWindow volumeMixerWindow: VolumeMixerWindow {}
    property OnboardingWindow onboarding: OnboardingWindow {}
    property UpdateDialog updateDialog: UpdateDialog {}
    property TaskbarWidget taskbarWidget: TaskbarWidget {
        stack: widgetStack
        onActivated: flyout.showFlyout()
    }
    property PeripheralsWidget peripheralsWidget: PeripheralsWidget {
        mediaWidget: taskbarWidget
        stack: widgetStack
    }
    property WidgetStack widgetStack: WidgetStack {
        mediaWidget: app.taskbarWidget
        peripheralsWidget: app.peripheralsWidget
    }

    Component.onCompleted: if(!settings.onboardingCompleted) onboarding.show()

    property Timer _updateTimer: Timer {
        interval: 4000
        running: settings.autoCheckUpdates
        onTriggered: updateChecker.check()
    }

    property Connections _updateConn: Connections {
        target: updateChecker
        function onUpdateFound(version) {
            if(version !== settings.skippedUpdateVersion)
                updateDialog.open()
        }
    }

    readonly property var flyoutStack: [flyout, volumeFlyout, lockKeysFlyout, nextUpFlyout, peripheralFlyout, chargerFlyout]

    function positionIndexFor(f) {
        if(f === volumeFlyout)
            return settings.volumeFlyoutPosition
        if(f === peripheralFlyout)
            return settings.peripheralFlyoutPosition
        if(f === chargerFlyout)
            return settings.chargerFlyoutPosition
        return settings.flyoutPosition
    }
    function monitorFor(f) {
        if(f === volumeFlyout)
            return settings.volumeFlyoutMonitor
        if(f === peripheralFlyout)
            return settings.peripheralFlyoutMonitor
        if(f === chargerFlyout)
            return settings.chargerFlyoutMonitor
        return settings.flyoutMonitor
    }

    function restack() {
        var gap = 10
        var used = ({})

        for(var i = 0; i < flyoutStack.length; i++) {
            var f = flyoutStack[i]
            if(!f.visible) {
                f.positioned = false
                continue
            }

            var posIndex = positionIndexFor(f)
            var monitor = monitorFor(f)
            var key = posIndex + ":" + monitor
            var cumulative = used[key] || 0
            var aboveWidget = posIndex === 6 // Dock above taskbar widget.
            var topAligned = posIndex <= 2 // 0..2 are the top positions

            if(aboveWidget) {
                var anchor = taskbar.widgetPosition(settings.taskbarWidgetMonitor, settings.taskbarWidgetSide,
                                                    settings.taskbarWidgetOffset, taskbarWidget.width,
                                                    taskbarWidget.height)
                var screen = monitors.geometry(settings.taskbarWidgetMonitor)
                var fx = anchor.x + (taskbarWidget.width - f.width) / 2
                fx = Math.max(screen.x + 8, Math.min(fx, screen.x + screen.width - f.width - 8))
                f.placeAt(fx, anchor.y - f.height - 12 - cumulative)
            } else {
                var p = placement.positionAt(f.width, f.height, posIndex, monitor)
                f.placeAt(p.x, topAligned ? (p.y + cumulative) : (p.y - cumulative))
            }
            used[key] = cumulative + f.height + gap
            f.positioned = true
        }
    }

    property Connections settingsConnection: Connections {
        target: settings
        function onChanged() { app.restack() }
    }

    property Connections monitorConnection: Connections {
        target: monitors
        function onChanged() {
            log.debug("App: monitor layout changed, restacking " + flyoutStack.length + " flyouts")
            app.restack()
        }
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

        function onOpenMixerRequested() {
            volumeMixerWindow.open()
        }

        function onReportBugRequested() {
            dashboard.showPage("about")
        }

        function onRestartRequested() {
            restarter.restart()
        }
    }

    property Connections instanceConnection: Connections {
        target: instance
        function onActivated() {
            if(!settings.trayIconHidden)
                return
            dashboard.show()
            dashboard.raise()
            dashboard.requestActivate()
        }
    }
}
