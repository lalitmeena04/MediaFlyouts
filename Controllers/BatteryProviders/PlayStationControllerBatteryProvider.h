#pragma once

#include "IBatteryProvider.h"

// Uses same report layout as DS4Windows.
class PlayStationControllerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
