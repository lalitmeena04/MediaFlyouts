#pragma once

#include "IBatteryProvider.h"

// Read game controller battery through Windows.Gaming.Input.
// Works with Bluetooth, USB dongle, and wired transport.
class GameControllerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
