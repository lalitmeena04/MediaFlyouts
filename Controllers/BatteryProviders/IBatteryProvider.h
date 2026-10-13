#pragma once

#include <QString>
#include <QVector>

struct BatteryComponentInfo {
        QString label; // "L", "R", "Case", ...
        int percent = -1;
        bool charging = false;
};

struct BatteryDeviceInfo {
        QString name;
        int percent = -1;
        // "bluetooth", "ble", "gamepad". Icon fallback in QML.
        QString connectionKind;
        QString category;
        bool charging = false;
        // Field percent stores the lowest component.
        QVector<BatteryComponentInfo> components;
};

class IBatteryProvider {
    public:
        virtual ~IBatteryProvider() = default;

        virtual QVector<BatteryDeviceInfo> poll() = 0;
};
