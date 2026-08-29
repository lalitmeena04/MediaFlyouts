#pragma once

#include <QHash>
#include <QString>
#include <QVector>

// Decides which peripheral events are worth showing. Kept free of Windows/Qt-GUI types
// so the rules can be unit tested without real hardware.
namespace mf {

// Battery buckets, aligned with BatteryRing's colours so the label and the ring agree.
enum class BatteryBucket { Unknown, VeryLow, Low, Half, Good, Full };

enum class PeripheralEvent { Connected, Disconnected, LowBattery, VeryLowBattery, ChargingStarted, ChargingStopped };

struct PeripheralAlert {
        PeripheralEvent event;
        QString deviceName;
        int percent = -1;
        BatteryBucket bucket = BatteryBucket::Unknown;
        // carried through so the card can show the right icon and link type
        QString category;
        QString connectionKind;
};

// One device as seen in a poll snapshot.
struct PeripheralSnapshot {
        QString name;
        int percent = -1;
        bool charging = false;
        QString category;
        QString connectionKind;
};

BatteryBucket bucketOf(int percent);
QString bucketLabel(BatteryBucket bucket);

// Tracks device state across snapshots and reports only the transitions worth alerting on.
class BatteryAlertPolicy {
    public:
        // Thresholds are the levels a device must cross *downward* to alert.
        static constexpr int kLowThreshold = 20;
        static constexpr int kVeryLowThreshold = 10;

        // The first snapshot only establishes a baseline: on launch every already-connected
        // device would otherwise look newly connected.
        QVector<PeripheralAlert> update(const QVector<PeripheralSnapshot>& snapshot, qint64 timestampMs);

        // Repeated connect/disconnect of the same device inside this window collapses into
        // one alert, so a flapping Bluetooth link doesn't spam the screen.
        void setDebounceMs(qint64 ms) { m_debounceMs = ms; }

        bool hasBaseline() const { return m_seeded; }

    private:
        struct DeviceState {
                int percent = -1;
                bool charging = false;
                bool lowAlerted = false;
                bool veryLowAlerted = false;
                // kept briefly after a disconnect so an immediate reconnect can be debounced
                bool present = true;
                qint64 lastConnectionEventMs = 0;
                QString category;
                QString connectionKind;
        };

        QHash<QString, DeviceState> m_devices;
        bool m_seeded = false;
        qint64 m_debounceMs = 3000;
};

} // namespace mf
