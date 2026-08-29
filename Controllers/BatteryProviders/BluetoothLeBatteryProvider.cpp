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

// GATT Characteristics UUIDs table doesn't expose a helper for Appearance (0x2A01);
// build the standard 16-bit-derived UUID manually (Bluetooth Base UUID).
constexpr winrt::guid kAppearanceCharacteristicUuid{
    0x00002a01, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb}};

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

// Reads the standard GAP Appearance characteristic (0x2A01) off the Generic Access
// Service (0x1800) and decodes it into a rough device category, per the Bluetooth SIG
// "GAP Appearance Values" assigned numbers. Returns an empty string if unavailable/unmapped.
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

            // DeviceInformation.Name is the paired/user-visible name shown in Windows
            // Bluetooth settings; BluetoothLEDevice.Name can fall back to a generic
            // "Bluetooth <address>" placeholder for devices that don't advertise a name
            QString name = QString::fromWCharArray(info.Name().c_str());
            if(name.isEmpty())
                name = QString::fromWCharArray(device.Name().c_str());
            if(name.isEmpty())
                continue;

            // still reported (with an unknown battery) when there's no Battery Service, so
            // connect/disconnect events surface even for devices that don't share battery
            result.append({name, percent, QStringLiteral("ble"), readCategory(device)});
        }
    } catch(...) {
        // BLE GATT access can fail transiently (device out of range, radio busy, ...); skip this cycle
    }

    return result;
}
