#pragma once

#include "IBatteryProvider.h"

#include <QHash>

// Reads battery from Razer wireless devices over their 90-byte HID feature-report protocol.
// Only devices that answer the power-class battery command are reported.
class RazerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        // transaction id each product answered on last time; models differ, and sweeping
        // the whole list costs a timeout per unanswered id
        QHash<quint16, quint8> m_transactionIds;
};
