#include "BatteryAlertPolicy.h"

#include <QSet>

namespace mf {

BatteryBucket bucketOf(int percent) {
    if(percent < 0)
        return BatteryBucket::Unknown;
    if(percent <= 10)
        return BatteryBucket::VeryLow;
    if(percent <= 20)
        return BatteryBucket::Low;
    if(percent <= 40)
        return BatteryBucket::Half;
    if(percent <= 89)
        return BatteryBucket::Good;
    return BatteryBucket::Full;
}

QString bucketLabel(BatteryBucket bucket) {
    switch(bucket) {
    case BatteryBucket::VeryLow: return QStringLiteral("Very low");
    case BatteryBucket::Low:     return QStringLiteral("Low");
    case BatteryBucket::Half:    return QStringLiteral("Half");
    case BatteryBucket::Good:    return QStringLiteral("Good");
    case BatteryBucket::Full:    return QStringLiteral("Full");
    case BatteryBucket::Unknown: break;
    }
    return QStringLiteral("Unknown");
}

QVector<PeripheralAlert> BatteryAlertPolicy::update(const QVector<PeripheralSnapshot>& snapshot,
                                                     qint64 timestampMs) {
    QVector<PeripheralAlert> alerts;
    QSet<QString> present;

    for(const PeripheralSnapshot& device : snapshot) {
        present.insert(device.name);
        const bool known = m_devices.contains(device.name);
        DeviceState state = m_devices.value(device.name);
        const bool reappeared = known && !state.present;

        if((!known || (reappeared && state.disconnectReported)) && m_seeded) {
            if(timestampMs - state.lastConnectionEventMs >= m_debounceMs) {
                alerts.append({PeripheralEvent::Connected, device.name, device.percent, bucketOf(device.percent),
                               device.category, device.connectionKind});
                state.lastConnectionEventMs = timestampMs;
            }
        }
        state.present = true;
        state.disconnectReported = false;
        state.absentSinceMs = 0;

        if(known && !reappeared && device.charging != state.charging) {
            alerts.append({device.charging ? PeripheralEvent::ChargingStarted : PeripheralEvent::ChargingStopped,
                           device.name, device.percent, bucketOf(device.percent), device.category,
                           device.connectionKind});
        }

        // Charging above threshold arms alert again.
        // Device alerts once per discharge.
        if(device.percent > kLowThreshold)
            state.lowAlerted = false;
        if(device.percent > kVeryLowThreshold)
            state.veryLowAlerted = false;

        // Only a downward crossing counts, and only while not charging.
        if(m_seeded && !device.charging && device.percent >= 0) {
            if(device.percent <= kVeryLowThreshold && !state.veryLowAlerted) {
                alerts.append({PeripheralEvent::VeryLowBattery, device.name, device.percent,
                               bucketOf(device.percent), device.category, device.connectionKind});
                state.veryLowAlerted = true;
                state.lowAlerted = true; // Do not also fire low warning.
            } else if(device.percent <= kLowThreshold && !state.lowAlerted) {
                alerts.append({PeripheralEvent::LowBattery, device.name, device.percent, bucketOf(device.percent),
                               device.category, device.connectionKind});
                state.lowAlerted = true;
            }
        }

        state.percent = device.percent;
        state.charging = device.charging;
        state.category = device.category;
        state.connectionKind = device.connectionKind;
        m_devices.insert(device.name, state);
    }

    for(auto it = m_devices.begin(); it != m_devices.end();) {
        if(present.contains(it.key())) {
            ++it;
            continue;
        }
        if(!it.value().present) {
            if(!it.value().disconnectReported &&
               timestampMs - it.value().absentSinceMs >= m_debounceMs) {
                // Report last known level.
                alerts.append({PeripheralEvent::Disconnected, it.key(), it.value().percent,
                               bucketOf(it.value().percent), it.value().category, it.value().connectionKind});
                it.value().disconnectReported = true;
                it.value().lastConnectionEventMs = timestampMs;
            } else if(it.value().disconnectReported &&
                      timestampMs - it.value().lastConnectionEventMs >= m_debounceMs) {
                it = m_devices.erase(it);
            } else {
                ++it;
            }
            continue;
        }
        it.value().present = false;
        it.value().absentSinceMs = timestampMs;
        ++it;
    }

    m_seeded = true;
    return alerts;
}

} // namespace mf
