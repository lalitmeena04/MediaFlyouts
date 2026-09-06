#include "BluetoothConnectionWatcher.h"

#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Enumeration;

BluetoothConnectionWatcher::BluetoothConnectionWatcher(QObject* parent) : QObject(parent) {}

BluetoothConnectionWatcher::~BluetoothConnectionWatcher() {
    try {
        if(m_classicWatcher)
            m_classicWatcher.Stop();
        if(m_leWatcher)
            m_leWatcher.Stop();
    } catch(...) {
    }
}

void BluetoothConnectionWatcher::onDeviceEvent() {
    QMetaObject::invokeMethod(this, [this] { emit deviceChanged(); }, Qt::QueuedConnection);
}

void BluetoothConnectionWatcher::attach(DeviceWatcher& watcher) {
    // Any event means Bluetooth device changed. Re-poll. Overlap is guarded.
    watcher.Added([this](const DeviceWatcher&, const DeviceInformation&) { onDeviceEvent(); });
    watcher.Removed([this](const DeviceWatcher&, const DeviceInformationUpdate&) { onDeviceEvent(); });
    watcher.Updated([this](const DeviceWatcher&, const DeviceInformationUpdate&) { onDeviceEvent(); });
    watcher.Start();
}

void BluetoothConnectionWatcher::start() {
    // Request this property. Updated fires on connect/disconnect only with it.
    winrt::Windows::Foundation::Collections::IVector<winrt::hstring> extraProperties =
        winrt::single_threaded_vector<winrt::hstring>({L"System.Devices.Aep.IsConnected"});

    try {
        m_classicWatcher = DeviceInformation::CreateWatcher(BluetoothDevice::GetDeviceSelectorFromPairingState(true),
                                                             extraProperties);
        attach(m_classicWatcher);
    } catch(...) {
        // Watcher creation can fail without Bluetooth radio. Timer polling still works.
    }

    try {
        m_leWatcher = DeviceInformation::CreateWatcher(BluetoothLEDevice::GetDeviceSelectorFromPairingState(true),
                                                        extraProperties);
        attach(m_leWatcher);
    } catch(...) {
    }
}
