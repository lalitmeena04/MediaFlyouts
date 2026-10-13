#pragma once

#include "IBatteryProvider.h"

class BluetoothClassicBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
