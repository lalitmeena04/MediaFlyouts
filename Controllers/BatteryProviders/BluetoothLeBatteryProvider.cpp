#include "BluetoothLeBatteryProvider.h"

#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace winrt::Windows::Devices::Enumeration;
namespace streams = winrt::Windows::Storage::Streams;

namespace {

// Reads the standard Battery Level characteristic (0x2A19) off the given device's
// Battery Service (0x180F); returns -1 if the device doesn't expose one.
int readBatteryPercent(const BluetoothLEDevice& device) {
    auto serviceResult = device.GetGattServicesForUuidAsync(GattServiceUuids::Battery(), BluetoothCacheMode::Uncached)
                              .get();
    if(serviceResult.Status() != GattCommunicationStatus::Success || serviceResult.Services().Size() == 0)
        return -1;

    auto service = serviceResult.Services().GetAt(0);
    auto charResult =
        service.GetCharacteristicsForUuidAsync(GattCharacteristicUuids::BatteryLevel(), BluetoothCacheMode::Uncached)
            .get();
    if(charResult.Status() != GattCommunicationStatus::Success || charResult.Characteristics().Size() == 0)
        return -1;

    auto characteristic = charResult.Characteristics().GetAt(0);
    auto readResult = characteristic.ReadValueAsync(BluetoothCacheMode::Uncached).get();
    if(readResult.Status() != GattCommunicationStatus::Success)
        return -1;

    streams::DataReader reader = streams::DataReader::FromBuffer(readResult.Value());
    if(reader.UnconsumedBufferLength() == 0)
        return -1;
    return static_cast<int>(reader.ReadByte());
}

} // namespace

QVector<BatteryDeviceInfo> BluetoothLeBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    try {
        const auto selector = BluetoothLEDevice::GetDeviceSelector();
        DeviceInformationCollection devices = DeviceInformation::FindAllAsync(selector).get();

        for(const DeviceInformation& info : devices) {
            BluetoothLEDevice device = BluetoothLEDevice::FromIdAsync(info.Id()).get();
            if(!device || device.ConnectionStatus() != BluetoothConnectionStatus::Connected)
                continue;

            const int percent = readBatteryPercent(device);
            if(percent < 0)
                continue;

            const QString name = QString::fromWCharArray(device.Name().c_str());
            if(name.isEmpty())
                continue;

            result.append({name, percent, QStringLiteral("ble")});
        }
    } catch(...) {
        // BLE GATT access can fail transiently (device out of range, radio busy, ...); skip this cycle
    }

    return result;
}
