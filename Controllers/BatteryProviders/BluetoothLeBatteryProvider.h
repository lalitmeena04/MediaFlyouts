#pragma once

#include "IBatteryProvider.h"

class BluetoothLeBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
