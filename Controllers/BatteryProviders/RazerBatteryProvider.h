#pragma once

#include "IBatteryProvider.h"

#include <QHash>

class RazerBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        QHash<quint16, quint8> m_transactionIds;
};
