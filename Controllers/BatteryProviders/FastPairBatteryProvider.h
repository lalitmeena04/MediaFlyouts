#pragma once

#include "IBatteryProvider.h"

#include <QHash>
#include <QString>

#include <atomic>
#include <mutex>
#include <thread>

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
        QHash<quint64, BatteryDeviceInfo> m_latest;
};
