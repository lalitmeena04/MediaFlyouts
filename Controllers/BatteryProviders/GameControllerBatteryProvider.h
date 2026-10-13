#pragma once

#include "IBatteryProvider.h"

class GameControllerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
