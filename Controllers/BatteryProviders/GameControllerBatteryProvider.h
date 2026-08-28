#pragma once

#include "IBatteryProvider.h"

// Reads battery level for game controllers (Xbox pads, DualSense, generic HID
// gamepads, ...) via the official Windows.Gaming.Input battery API. Works regardless
// of transport (Bluetooth, USB dongle, wired), unlike the other providers.
class GameControllerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
