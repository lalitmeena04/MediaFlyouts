#pragma once

#include "IBatteryProvider.h"

#include <QHash>
#include <QSet>
#include <QString>

class LogitechBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        // A receiver goes silent while all its devices sleep. Skip is not permanent.
        QHash<QString, int> m_skipPolls;
        // Skips reset when collections appear or vanish. A channel switch changes this set.
        QSet<QString> m_lastPaths;
};
