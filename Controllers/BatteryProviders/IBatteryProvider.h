#pragma once

#include <QString>
#include <QVector>

// One battery of multi-battery device.
struct BatteryComponentInfo {
        QString label; // "L", "R", "Case", ...
        int percent = -1;
        bool charging = false;
};

// One discovered peripheral with a readable battery level.
struct BatteryDeviceInfo {
        QString name;
        int percent = -1;
        // "bluetooth", "ble", "gamepad". Icon fallback in QML.
        QString connectionKind;
        // Device category. Empty if unknown. Prefer decoded class over name guess.
        QString category;
        bool charging = false;
        // Populated only for multi-battery devices.
        // Field percent then stores lowest component.
        QVector<BatteryComponentInfo> components;
};

// Battery info source for one peripheral connection class.
// Return devices with unknown battery too. Connect/disconnect events stay visible.
class IBatteryProvider {
    public:
        virtual ~IBatteryProvider() = default;

        // Synchronous snapshot of readable battery levels.
        // BatteryStatusController calls this from timer.
        virtual QVector<BatteryDeviceInfo> poll() = 0;
};
