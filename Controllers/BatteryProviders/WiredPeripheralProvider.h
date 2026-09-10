#pragma once

#include "IBatteryProvider.h"

// Report wired USB peripherals. Windows-only.
// These devices have no readable battery, so percent stays -1.
// They still produce connect and disconnect events.
// Data storage is filtered out by mf::isAnnounceableWiredDevice.
class WiredPeripheralProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
