#pragma once

#include "IBatteryProvider.h"

// Read BLE battery from standard GATT Battery Service.
// Classic Bluetooth PnP property does not cover BLE.
class BluetoothLeBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
