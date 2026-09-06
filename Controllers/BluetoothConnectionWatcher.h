#pragma once

#include <QObject>

#include <winrt/Windows.Devices.Enumeration.h>

// Watch Bluetooth devices connect and disconnect.
// BatteryStatusController reacts before next poll.
class BluetoothConnectionWatcher : public QObject {
        Q_OBJECT

    public:
        explicit BluetoothConnectionWatcher(QObject* parent = nullptr);
        ~BluetoothConnectionWatcher() override;

        void start();

    signals:
        // Device was added, removed, or changed.
        void deviceChanged();

    private:
        void attach(winrt::Windows::Devices::Enumeration::DeviceWatcher& watcher);
        void onDeviceEvent();

        winrt::Windows::Devices::Enumeration::DeviceWatcher m_classicWatcher{nullptr};
        winrt::Windows::Devices::Enumeration::DeviceWatcher m_leWatcher{nullptr};
};
