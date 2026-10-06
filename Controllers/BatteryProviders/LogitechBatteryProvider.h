#pragma once

#include "IBatteryProvider.h"

#include <QHash>
#include <QSet>
#include <QString>

// Read Logitech battery over HID++ 2.0.
// Covers direct devices and receiver slots.
class LogitechBatteryProvider : public IBatteryProvider {
    public:
        QVector<BatteryDeviceInfo> poll() override;

    private:
        // Silent collections are skipped for a few polls. Avoid repeated timeouts.
        // Not permanent: a receiver goes silent while all its devices sleep.
        QHash<QString, int> m_skipPolls;
        // Skips reset when collections appear or vanish. A channel switch changes this set.
        QSet<QString> m_lastPaths;
};
