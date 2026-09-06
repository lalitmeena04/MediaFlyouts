#pragma once

#include "IBatteryProvider.h"

// Read Sony PlayStation controller battery from raw HID input report.
// Uses same report layout as DS4Windows.
class PlayStationControllerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
