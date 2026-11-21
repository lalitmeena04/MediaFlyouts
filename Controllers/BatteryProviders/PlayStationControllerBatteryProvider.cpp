#include "PlayStationControllerBatteryProvider.h"

#include "DualSenseProtocol.h"
#include "BatteryProviders/HidCollections.h"
#include "Win32Util.h"

#include <windows.h>

#include <vector>

namespace {

constexpr USHORT kSonyVendorId = 0x054C;
constexpr USHORT kDualSenseProductId = 0x0CE6;
constexpr USHORT kDualSenseEdgeProductId = 0x0DF2;

bool readInputReport(HANDLE device, std::vector<BYTE>& buffer) {
    OVERLAPPED ov = {};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if(!ov.hEvent)
        return false;

    bool ok = false;
    DWORD bytesRead = 0;
    if(ReadFile(device, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, &ov)) {
        ok = bytesRead > 0;
    } else if(GetLastError() == ERROR_IO_PENDING) {
        if(WaitForSingleObject(ov.hEvent, 500) == WAIT_OBJECT_0)
            ok = GetOverlappedResult(device, &ov, &bytesRead, FALSE) && bytesRead > 0;
        else
            CancelIoEx(device, &ov);
    }
    CloseHandle(ov.hEvent);
    return ok;
}

bool writeReport(HANDLE device, const std::vector<BYTE>& buffer) {
    OVERLAPPED ov = {};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if(!ov.hEvent)
        return false;

    bool ok = false;
    DWORD bytesWritten = 0;
    if(WriteFile(device, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesWritten, &ov)) {
        ok = true;
    } else if(GetLastError() == ERROR_IO_PENDING) {
        if(WaitForSingleObject(ov.hEvent, 500) == WAIT_OBJECT_0)
            ok = GetOverlappedResult(device, &ov, &bytesWritten, FALSE);
        else
            CancelIoEx(device, &ov);
    }
    CloseHandle(ov.hEvent);
    return ok;
}

// Bluetooth DualSense starts with simple input report id 0x01.
// Send signed report to enable extended report id 0x31.
bool enableExtendedBluetoothReport(HANDLE device) {
    return writeReport(device, DualSenseProtocol::buildEnableExtendedReportPacket());
}

} // namespace

QVector<BatteryDeviceInfo> PlayStationControllerBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    for(const HidCollection& collection : enumerateHidCollections(kSonyVendorId)) {
        if(collection.productId != kDualSenseProductId
           && collection.productId != kDualSenseEdgeProductId)
            continue;
        if(collection.inputLength == 0)
            continue;

        const std::wstring devicePath = collection.path.toStdWString();
        HANDLE rawHandle = CreateFileW(devicePath.c_str(),
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                     FILE_FLAG_OVERLAPPED, nullptr);
        if(rawHandle == INVALID_HANDLE_VALUE)
            continue;
        mf::ScopedHandle handle{rawHandle};

        const bool isBluetooth = collection.inputLength != 64;

        if(isBluetooth)
            enableExtendedBluetoothReport(handle.get());

        std::vector<BYTE> report(collection.inputLength);
        int percent = -1;
        for(int attempt = 0; attempt < (isBluetooth ? 5 : 1); ++attempt) {
            if(!readInputReport(handle.get(), report))
                break;
            percent = DualSenseProtocol::parseBatteryPercent(report, isBluetooth);
            if(percent >= 0)
                break;
        }
        if(percent < 0)
            continue;

        result.append({collection.productId == kDualSenseEdgeProductId ? QStringLiteral("DualSense Edge Controller")
                                                                       : QStringLiteral("DualSense Controller"),
                       percent, isBluetooth ? QStringLiteral("bluetooth") : QStringLiteral("usb"),
                       QStringLiteral("gamepad")});
    }

    return result;
}
