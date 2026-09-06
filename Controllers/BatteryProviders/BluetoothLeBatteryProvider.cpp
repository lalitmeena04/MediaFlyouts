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

// GATT UUID table has no Appearance helper. Build Bluetooth Base UUID manually.
constexpr winrt::guid kAppearanceCharacteristicUuid{
    0x00002a01, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb}};

// Read Battery Level characteristic from Battery Service. Return -1 if absent.
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

// Read GAP Appearance from Generic Access Service. Map to rough device category.
// Return empty string if absent or unmapped.
QString readCategory(const BluetoothLEDevice& device) {
    try {
        auto serviceResult =
            device.GetGattServicesForUuidAsync(GattServiceUuids::GenericAccess(), BluetoothCacheMode::Uncached).get();
        if(serviceResult.Status() != GattCommunicationStatus::Success || serviceResult.Services().Size() == 0)
            return {};

        auto service = serviceResult.Services().GetAt(0);
        auto charResult =
            service.GetCharacteristicsForUuidAsync(kAppearanceCharacteristicUuid, BluetoothCacheMode::Uncached).get();
        if(charResult.Status() != GattCommunicationStatus::Success || charResult.Characteristics().Size() == 0)
            return {};

        auto characteristic = charResult.Characteristics().GetAt(0);
        auto readResult = characteristic.ReadValueAsync(BluetoothCacheMode::Uncached).get();
        if(readResult.Status() != GattCommunicationStatus::Success)
            return {};

        streams::DataReader reader = streams::DataReader::FromBuffer(readResult.Value());
        reader.ByteOrder(streams::ByteOrder::LittleEndian);
        if(reader.UnconsumedBufferLength() < 2)
            return {};
        const quint16 appearance = reader.ReadUInt16();

        const quint16 category = appearance >> 6;
        const quint16 subcategory = appearance & 0x3F;
        if(category == 0x0F) { // Human Interface Device
            if(subcategory == 1)
                return QStringLiteral("keyboard");
            if(subcategory == 2)
                return QStringLiteral("mouse");
            if(subcategory == 3 || subcategory == 4)
                return QStringLiteral("gamepad"); // joystick / gamepad
        } else if(category == 0x0A) { // Audio
            return QStringLiteral("headset");
        }
        return {};
    } catch(...) {
        return {};
    }
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

            // DeviceInformation.Name is paired name from Windows Bluetooth settings.
            // BluetoothLEDevice.Name can be generic placeholder.
            QString name = QString::fromWCharArray(info.Name().c_str());
            if(name.isEmpty())
                name = QString::fromWCharArray(device.Name().c_str());
            if(name.isEmpty())
                continue;

            // Report devices without Battery Service as unknown battery.
            // This keeps connect/disconnect events visible.
            result.append({name, percent, QStringLiteral("ble"), readCategory(device)});
        }
    } catch(...) {
        // BLE GATT access can fail transiently. Skip this cycle.
    }

    return result;
}
