#pragma once

#include <QString>
#include <QVector>

// One discovered peripheral with a readable battery level.
struct BatteryDeviceInfo {
        QString name;
        int percent = -1;
        // "bluetooth", "ble", "gamepad", ... — used to pick an icon in QML
        QString connectionKind;
};

// A source of battery info for one class of peripheral connection (Bluetooth Classic,
// BLE, game controllers, ...). Implementations only return devices they can actually
// read a battery level for.
class IBatteryProvider {
    public:
        virtual ~IBatteryProvider() = default;

        // Synchronous snapshot of currently readable battery levels. Called periodically
        // by BatteryStatusController off a timer.
        virtual QVector<BatteryDeviceInfo> poll() = 0;
};
