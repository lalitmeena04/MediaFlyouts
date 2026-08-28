#pragma once

#include "IBatteryProvider.h"

// Reads battery level for paired classic-Bluetooth peripherals (headsets, mice,
// keyboards) via the same PnP device property Windows Settings uses.
class BluetoothClassicBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
