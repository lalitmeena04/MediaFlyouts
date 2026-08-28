#pragma once

#include <QObject>

#include <winrt/Windows.Devices.Enumeration.h>

// Watches for Bluetooth (classic + BLE) devices connecting/disconnecting so
// BatteryStatusController can react immediately instead of waiting for the next poll.
class BluetoothConnectionWatcher : public QObject {
        Q_OBJECT

    public:
        explicit BluetoothConnectionWatcher(QObject* parent = nullptr);
        ~BluetoothConnectionWatcher() override;

        void start();

    signals:
        // a device was added, removed, or had a property (e.g. connection state) change
        void deviceChanged();

    private:
        void attach(winrt::Windows::Devices::Enumeration::DeviceWatcher& watcher);
        void onDeviceEvent();

        winrt::Windows::Devices::Enumeration::DeviceWatcher m_classicWatcher{nullptr};
        winrt::Windows::Devices::Enumeration::DeviceWatcher m_leWatcher{nullptr};
};
