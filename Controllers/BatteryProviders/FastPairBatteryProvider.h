#pragma once

#include "IBatteryProvider.h"

#include <QHash>
#include <QString>

#include <atomic>
#include <mutex>
#include <thread>

// Reads battery over the Google Fast Pair "message stream" (RFCOMM). Unlike the classic
// Bluetooth provider, which depends on the Hands-Free profile, this works for devices
// with no microphone and reports each component of a true-wireless set separately.
//
// Devices push updates whenever a level or charging state changes, so this keeps a
// socket open per device on its own thread; poll() just returns the latest snapshot.
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
        // latest reading per device address, guarded by m_mutex
        QHash<quint64, BatteryDeviceInfo> m_latest;
};
