#pragma once

#include <QHash>
#include <QString>
#include <QVector>

namespace mf {

// Battery buckets align with BatteryRing colors.
enum class BatteryBucket { Unknown, VeryLow, Low, Half, Good, Full };

enum class PeripheralEvent { Connected, Disconnected, LowBattery, VeryLowBattery, ChargingStarted, ChargingStopped };

struct PeripheralAlert {
        PeripheralEvent event;
        QString deviceName;
        int percent = -1;
        BatteryBucket bucket = BatteryBucket::Unknown;
        QString category;
        QString connectionKind;
};

struct PeripheralSnapshot {
        QString name;
        int percent = -1;
        bool charging = false;
        QString category;
        QString connectionKind;
};

BatteryBucket bucketOf(int percent);
QString bucketLabel(BatteryBucket bucket);

class BatteryAlertPolicy {
    public:
        // Thresholds are the levels a device must cross *downward* to alert.
        static constexpr int kLowThreshold = 20;
        static constexpr int kVeryLowThreshold = 10;

        QVector<PeripheralAlert> update(const QVector<PeripheralSnapshot>& snapshot, qint64 timestampMs);

        // Prevent alert spam from unstable Bluetooth links.
        void setDebounceMs(qint64 ms) { m_debounceMs = ms; }

        bool hasBaseline() const { return m_seeded; }

    private:
        struct DeviceState {
                int percent = -1;
                bool charging = false;
                bool lowAlerted = false;
                bool veryLowAlerted = false;
                // Keep briefly after missing snapshot. Ignore transient provider gaps.
                bool present = true;
                bool disconnectReported = false;
                qint64 absentSinceMs = 0;
                qint64 lastConnectionEventMs = 0;
                QString category;
                QString connectionKind;
        };

        QHash<QString, DeviceState> m_devices;
        bool m_seeded = false;
        qint64 m_debounceMs = 3000;
};

} // namespace mf
