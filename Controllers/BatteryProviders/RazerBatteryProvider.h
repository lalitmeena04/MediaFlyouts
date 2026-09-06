#pragma once

#include "IBatteryProvider.h"

#include <QHash>

// Read Razer wireless battery over 90-byte HID feature-report protocol.
// Report only devices that answer power-class battery command.
class RazerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        // Last transaction id per product. Avoid full sweep timeout.
        QHash<quint16, quint8> m_transactionIds;
};
