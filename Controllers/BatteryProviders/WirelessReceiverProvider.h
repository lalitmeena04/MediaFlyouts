#pragma once

#include "IBatteryProvider.h"
#include "ReceiverProfiles.h"

const mf::ReceiverProfile* wirelessReceiverProfile(quint16 vendorId, quint16 productId);

class WirelessReceiverProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;
};
