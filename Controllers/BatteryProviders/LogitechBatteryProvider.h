#pragma once

#include "IBatteryProvider.h"

#include <QSet>
#include <QString>

// Reads battery from Logitech devices over HID++ 2.0, covering both directly attached
// devices and those paired to a Unifying / Lightspeed / Bolt receiver.
class LogitechBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        // vendor collections that turned out not to speak HID++ (some G headsets); probing
        // them costs a timeout per device index, so only do it once
        QSet<QString> m_notHidpp;
};
