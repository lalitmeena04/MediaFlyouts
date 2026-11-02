#pragma once

#include <QObject>

#include "Entitlement.h"

class Settings;
class EntitlementController;

// Effective values of the gated settings. Stored setting when unlocked, free-tier
// default when locked. QML behaviour binds here; the Dashboard toggles stay on Settings.
class ProGate : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool visualizerEnabled READ visualizerEnabled NOTIFY changed)
        Q_PROPERTY(bool mixerEnabled READ mixerEnabled NOTIFY changed)
        Q_PROPERTY(bool mixerWindowAllowed READ mixerWindowAllowed NOTIFY changed)
        Q_PROPERTY(bool peripheralsWidgetEnabled READ peripheralsWidgetEnabled NOTIFY changed)
        Q_PROPERTY(bool peripheralFlyoutEnabled READ peripheralFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool peripheralLowBatteryToast READ peripheralLowBatteryToast NOTIFY changed)
        Q_PROPERTY(int nextUpStyle READ nextUpStyle NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetControlsEnabled READ taskbarWidgetControlsEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetPauseAllEnabled READ taskbarWidgetPauseAllEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetPlayAllEnabled READ taskbarWidgetPlayAllEnabled NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetLayout READ taskbarWidgetLayout NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetShowPauseOverlay READ taskbarWidgetShowPauseOverlay NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetScrollingEnabled READ taskbarWidgetScrollingEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetAdaptiveColors READ taskbarWidgetAdaptiveColors NOTIFY changed)

    public:
        ProGate(Settings* settings, EntitlementController* entitlement, QObject* parent = nullptr);

        bool visualizerEnabled() const;
        bool mixerEnabled() const;
        bool mixerWindowAllowed() const;
        bool peripheralsWidgetEnabled() const;
        bool peripheralFlyoutEnabled() const;
        bool peripheralLowBatteryToast() const;
        int nextUpStyle() const;
        bool taskbarWidgetControlsEnabled() const;
        bool taskbarWidgetPauseAllEnabled() const;
        bool taskbarWidgetPlayAllEnabled() const;
        int taskbarWidgetLayout() const;
        bool taskbarWidgetShowPauseOverlay() const;
        bool taskbarWidgetScrollingEnabled() const;
        bool taskbarWidgetAdaptiveColors() const;

        bool unlocked(mf::ProFeature feature) const;

    signals:
        void changed();

    private:
        Settings* m_settings;
        EntitlementController* m_entitlement;
};
