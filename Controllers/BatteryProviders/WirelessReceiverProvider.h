#pragma once

#include "IBatteryProvider.h"

struct WirelessReceiverProfile {
        quint16 vendorId;
        quint16 productId;
        QString name;
        QString category;
};

// Returns a known receiver profile, or nullptr when the receiver is unsupported.
const WirelessReceiverProfile* wirelessReceiverProfile(quint16 vendorId, quint16 productId);

// Reports known proprietary wireless receivers that cannot provide battery telemetry.
// Their presence still enables normal peripheral connect/disconnect alerts.
class WirelessReceiverProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
