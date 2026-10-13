#pragma once

#include "IBatteryProvider.h"

class WiredPeripheralProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
