#pragma once

#include "IBatteryProvider.h"

#include <QHash>
#include <QString>

#include <atomic>
#include <mutex>
#include <thread>

// Read battery over Google Fast Pair message stream.
// Works for devices without Hands-Free profile.
// Reports true-wireless components separately.
// Worker keeps socket open per device. Poll returns latest snapshot.
class FastPairBatteryProvider : public IBatteryProvider {
    public:
        FastPairBatteryProvider();
        ~FastPairBatteryProvider() override;

        QVector<BatteryDeviceInfo> poll() override;

    private:
        void run();

        std::thread m_thread;
        std::atomic<bool> m_stop{false};
        std::mutex m_mutex;
        // Latest reading per device address. Guarded by m_mutex.
        QHash<quint64, BatteryDeviceInfo> m_latest;
};
