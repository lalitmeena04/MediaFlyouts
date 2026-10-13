#pragma once

#include <QObject>

#include <winrt/Windows.Devices.Enumeration.h>

class BluetoothConnectionWatcher : public QObject {
        Q_OBJECT

    public:
        explicit BluetoothConnectionWatcher(QObject* parent = nullptr);
        ~BluetoothConnectionWatcher() override;

        void start();

    signals:
        void deviceChanged();

    private:
        void attach(winrt::Windows::Devices::Enumeration::DeviceWatcher& watcher);
        void onDeviceEvent();

        winrt::Windows::Devices::Enumeration::DeviceWatcher m_classicWatcher{nullptr};
        winrt::Windows::Devices::Enumeration::DeviceWatcher m_leWatcher{nullptr};
};
