#pragma once

#include <optional>

#include "BatteryAlertPolicy.h"

// Decide when a charger change is worth showing.
// Keep rules free of Windows and Qt GUI types.
namespace mf {

enum class PowerSource { Unknown, Battery, AC };

enum class ChargerEvent { PluggedIn, Unplugged };

// One reading of the system power state.
struct PowerSnapshot {
        PowerSource source = PowerSource::Unknown;
        // System battery charge. -1 when unknown.
        int percent = -1;
};

struct ChargerAlert {
        ChargerEvent event;
        int percent = -1;
        BatteryBucket bucket = BatteryBucket::Unknown;
};

// Track the power source across readings. Report plug and unplug only.
class PowerAlertPolicy {
    public:
        // First known reading sets the baseline and raises no alert.
        std::optional<ChargerAlert> update(const PowerSnapshot& snapshot);

        bool hasBaseline() const { return m_source != PowerSource::Unknown; }

    private:
        PowerSource m_source = PowerSource::Unknown;
};

} // namespace mf
