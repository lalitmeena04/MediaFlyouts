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
    // any of these firing means "something about a Bluetooth device changed" - good
    // enough to trigger a re-poll; BatteryStatusController::refresh() is cheap to call
    // spuriously (guarded against overlap), so we don't filter which property changed
    watcher.Added([this](const DeviceWatcher&, const DeviceInformation&) { onDeviceEvent(); });
    watcher.Removed([this](const DeviceWatcher&, const DeviceInformationUpdate&) { onDeviceEvent(); });
    watcher.Updated([this](const DeviceWatcher&, const DeviceInformationUpdate&) { onDeviceEvent(); });
    watcher.Start();
}

void BluetoothConnectionWatcher::start() {
    // requesting this property explicitly is required for Updated to actually fire on
    // connect/disconnect - without it the watcher only reports pair/unpair (verified empirically)
    winrt::Windows::Foundation::Collections::IVector<winrt::hstring> extraProperties =
        winrt::single_threaded_vector<winrt::hstring>({L"System.Devices.Aep.IsConnected"});

    try {
        m_classicWatcher = DeviceInformation::CreateWatcher(BluetoothDevice::GetDeviceSelectorFromPairingState(true),
                                                             extraProperties);
        attach(m_classicWatcher);
    } catch(...) {
        // watcher creation can fail if the Bluetooth radio is off/absent; battery polling
        // still works via the timer fallback, just without instant connect/disconnect
    }

    try {
        m_leWatcher = DeviceInformation::CreateWatcher(BluetoothLEDevice::GetDeviceSelectorFromPairingState(true),
                                                        extraProperties);
        attach(m_leWatcher);
    } catch(...) {
    }
}
