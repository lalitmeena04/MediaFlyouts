#pragma once

#include "IBatteryProvider.h"

#include <QSet>
#include <QString>

// Read Logitech battery over HID++ 2.0.
// Covers direct devices and receiver slots.
class LogitechBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        // Vendor collections can fail HID++ probe. Avoid repeated timeouts.
        QSet<QString> m_notHidpp;
};
