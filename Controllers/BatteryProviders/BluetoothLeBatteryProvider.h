#pragma once

#include "IBatteryProvider.h"

// Reads battery level for paired BLE peripherals (mice, keyboards, ...) via the
// standard GATT Battery Service (UUID 0x180F), which classic-Bluetooth's PnP
// property doesn't cover.
class BluetoothLeBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
