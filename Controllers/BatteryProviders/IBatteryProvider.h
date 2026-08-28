#pragma once

#include <QString>
#include <QVector>

// One battery of a multi-battery device, e.g. the left bud of a true-wireless set.
struct BatteryComponentInfo {
        QString label; // "L", "R", "Case", ...
        int percent = -1;
        bool charging = false;
};

// One discovered peripheral with a readable battery level.
struct BatteryDeviceInfo {
        QString name;
        int percent = -1;
        // "bluetooth", "ble", "gamepad", ... — used as an icon fallback in QML
        QString connectionKind;
        // "mouse", "keyboard", "headset", "gamepad", or empty if unknown - decoded from the
        // Bluetooth Class of Device where available, more reliable than guessing from the name
        QString category;
        bool charging = false;
        // populated only when the device reports more than one battery; 'percent' above is
        // then the lowest of these
        QVector<BatteryComponentInfo> components;
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
