import QtQuick
import MediaFlyouts

// Non-visual app root: owns the flyout + dashboard windows and reacts to tray intents.
QtObject {
    property MediaFlyout flyout: MediaFlyout {}
    property VolumeFlyout volumeFlyout: VolumeFlyout {}
    property LockKeysFlyout lockKeysFlyout: LockKeysFlyout {}
    property Dashboard dashboard: Dashboard {}

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
}
