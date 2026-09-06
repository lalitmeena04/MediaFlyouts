#pragma once

#include "IBatteryProvider.h"

struct WirelessReceiverProfile {
        quint16 vendorId;
        quint16 productId;
        QString name;
        QString category;
};

// Return known receiver profile. Return nullptr for unsupported receiver.
const WirelessReceiverProfile* wirelessReceiverProfile(quint16 vendorId, quint16 productId);

// Report known proprietary wireless receivers without battery telemetry.
// Presence still enables connect/disconnect alerts.
class WirelessReceiverProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
