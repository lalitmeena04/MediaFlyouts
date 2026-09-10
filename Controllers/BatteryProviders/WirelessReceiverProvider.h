#pragma once

#include "IBatteryProvider.h"
#include "ReceiverProfiles.h"

// Return known receiver profile. Return nullptr for unsupported receiver.
// Profiles come from the built-in list plus the user's receivers.json.
const mf::ReceiverProfile* wirelessReceiverProfile(quint16 vendorId, quint16 productId);

// Report known proprietary wireless receivers without battery telemetry.
// Presence still enables connect/disconnect alerts.
class WirelessReceiverProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
