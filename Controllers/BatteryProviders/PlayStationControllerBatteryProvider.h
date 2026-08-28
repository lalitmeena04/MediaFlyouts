#pragma once

#include "IBatteryProvider.h"

// Reads battery for Sony PlayStation controllers (DualSense today) via their raw HID
// input report. Windows.Gaming.Input doesn't expose battery for these (and is
// foreground-only besides), so this talks to the HID device directly instead,
// following the same report layout DS4Windows uses.
class PlayStationControllerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
