#include "ProGate.h"

#include "EntitlementController.h"
#include "Logging.h"
#include "Settings.h"

using mf::ProFeature;

ProGate::ProGate(Settings* settings, EntitlementController* entitlement, QObject* parent)
    : QObject(parent), m_settings(settings), m_entitlement(entitlement) {
    if(m_settings)
        connect(m_settings, &Settings::changed, this, &ProGate::changed);
    if(m_entitlement) {
        connect(m_entitlement, &EntitlementController::changed, this, &ProGate::changed);
        connect(m_entitlement, &EntitlementController::changed, this, [this] {
            TRC_DEBUG("pro gate: visualizer={} mixer={} peripherals={} styles={} widgetOptions={}",
                      unlocked(ProFeature::Visualizer), unlocked(ProFeature::PerAppMixer),
                      unlocked(ProFeature::PeripheralBattery), unlocked(ProFeature::NowPlayingStyles),
                      unlocked(ProFeature::TaskbarWidgetOptions));
        });
    }
}

bool ProGate::unlocked(ProFeature feature) const {
    // No controller = no paywall.
    return !m_entitlement || mf::isUnlocked(feature, m_entitlement->state().tier);
}

bool ProGate::visualizerEnabled() const {
    return m_settings && m_settings->taskbarVisualizerEnabled() && unlocked(ProFeature::Visualizer);
}

bool ProGate::mixerEnabled() const {
    return m_settings && m_settings->volumeFlyoutMixerEnabled() && unlocked(ProFeature::PerAppMixer);
}

bool ProGate::mixerWindowAllowed() const {
    return unlocked(ProFeature::PerAppMixer);
}

bool ProGate::peripheralsWidgetEnabled() const {
    return m_settings && m_settings->peripheralsWidgetEnabled() && unlocked(ProFeature::PeripheralBattery);
}

bool ProGate::peripheralFlyoutEnabled() const {
    return m_settings && m_settings->peripheralFlyoutEnabled() && unlocked(ProFeature::PeripheralBattery);
}

bool ProGate::peripheralLowBatteryToast() const {
    return m_settings && m_settings->peripheralLowBatteryToast() && unlocked(ProFeature::PeripheralBattery);
}

int ProGate::nextUpStyle() const {
    return m_settings && unlocked(ProFeature::NowPlayingStyles) ? m_settings->nextUpStyle() : 0;
}

bool ProGate::taskbarWidgetControlsEnabled() const {
    return m_settings && unlocked(ProFeature::TaskbarWidgetOptions) ? m_settings->taskbarWidgetControlsEnabled() : true;
}

bool ProGate::taskbarWidgetPauseAllEnabled() const {
    return m_settings && unlocked(ProFeature::TaskbarWidgetOptions) ? m_settings->taskbarWidgetPauseAllEnabled()
                                                                     : false;
}

bool ProGate::taskbarWidgetPlayAllEnabled() const {
    return m_settings && unlocked(ProFeature::TaskbarWidgetOptions) ? m_settings->taskbarWidgetPlayAllEnabled() : true;
}

int ProGate::taskbarWidgetLayout() const {
    return m_settings && unlocked(ProFeature::TaskbarWidgetOptions) ? m_settings->taskbarWidgetLayout() : 0;
}

bool ProGate::taskbarWidgetShowPauseOverlay() const {
    return m_settings && unlocked(ProFeature::TaskbarWidgetOptions) ? m_settings->taskbarWidgetShowPauseOverlay()
                                                                     : true;
}

bool ProGate::taskbarWidgetScrollingEnabled() const {
    return m_settings && unlocked(ProFeature::TaskbarWidgetOptions) ? m_settings->taskbarWidgetScrollingEnabled()
                                                                     : true;
}

bool ProGate::taskbarWidgetAdaptiveColors() const {
    return m_settings && unlocked(ProFeature::TaskbarWidgetOptions) ? m_settings->taskbarWidgetAdaptiveColors() : true;
}
