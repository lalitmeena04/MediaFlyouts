#pragma once

#include "IBatteryProvider.h"

// Read classic Bluetooth battery from same PnP property as Windows Settings.
// Also report connected devices with unknown battery.
class BluetoothClassicBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
