#pragma once

#include "IBatteryProvider.h"

#include <QHash>
#include <QString>

#include <cstdint>

// Jabra headsets reached through a Link dongle or on their own USB cable. Speaks GNP over HID.
class JabraBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        // Headset powered off leaves the dongle silent. Skip is not permanent.
        QHash<QString, int> m_skipPolls;
        std::uint8_t m_sequence = 0;
};

// True when a Jabra product exposes the GNP vendor collection. WiredPeripheralProvider leaves it to this provider.
bool jabraGnpProductPresent(quint16 productId);
