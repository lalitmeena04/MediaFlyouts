#pragma once

#include <optional>

#include "BatteryAlertPolicy.h"

namespace mf {

enum class PowerSource { Unknown, Battery, AC };

enum class ChargerEvent { PluggedIn, Unplugged };

struct PowerSnapshot {
        PowerSource source = PowerSource::Unknown;
        int percent = -1;
};

struct ChargerAlert {
        ChargerEvent event;
        int percent = -1;
        BatteryBucket bucket = BatteryBucket::Unknown;
};

class PowerAlertPolicy {
    public:
        std::optional<ChargerAlert> update(const PowerSnapshot& snapshot);

        bool hasBaseline() const { return m_source != PowerSource::Unknown; }

    private:
        PowerSource m_source = PowerSource::Unknown;
};

} // namespace mf
