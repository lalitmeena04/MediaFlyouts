#include "PlayStationControllerBatteryProvider.h"

#include "DualSenseProtocol.h"

#include <windows.h>

#include <hidsdi.h>

#include <hidpi.h>

#include <setupapi.h>

#include <vector>

namespace {

constexpr USHORT kSonyVendorId = 0x054C;
constexpr USHORT kDualSenseProductId = 0x0CE6;
constexpr USHORT kDualSenseEdgeProductId = 0x0DF2;

struct ScopedHandle {
        HANDLE handle = INVALID_HANDLE_VALUE;
        ~ScopedHandle() {
            if(handle != INVALID_HANDLE_VALUE)
                CloseHandle(handle);
        }
};

// Read one input report from open HID device. Use short timeout.
// Sleeping device must not block poll thread.
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

    GUID hidGuid = {};
    HidD_GetHidGuid(&hidGuid);

    HDEVINFO deviceInfo = SetupDiGetClassDevsW(&hidGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(deviceInfo == INVALID_HANDLE_VALUE)
        return result;

    SP_DEVICE_INTERFACE_DATA ifaceData = {};
    ifaceData.cbSize = sizeof(ifaceData);

    for(DWORD index = 0; SetupDiEnumDeviceInterfaces(deviceInfo, nullptr, &hidGuid, index, &ifaceData); ++index) {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetailW(deviceInfo, &ifaceData, nullptr, 0, &requiredSize, nullptr);
        if(requiredSize == 0)
            continue;

        std::vector<BYTE> detailBuffer(requiredSize);
        auto* detail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(detailBuffer.data());
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        if(!SetupDiGetDeviceInterfaceDetailW(deviceInfo, &ifaceData, detail, requiredSize, nullptr, nullptr))
            continue;

        HANDLE handle = CreateFileW(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                     FILE_FLAG_OVERLAPPED, nullptr);
        if(handle == INVALID_HANDLE_VALUE)
            continue;
        ScopedHandle guard{handle};

        HIDD_ATTRIBUTES attrs = {};
        attrs.Size = sizeof(attrs);
        if(!HidD_GetAttributes(handle, &attrs))
            continue;
        if(attrs.VendorID != kSonyVendorId
           || (attrs.ProductID != kDualSenseProductId && attrs.ProductID != kDualSenseEdgeProductId))
            continue;

        PHIDP_PREPARSED_DATA preparsed = nullptr;
        if(!HidD_GetPreparsedData(handle, &preparsed))
            continue;
        HIDP_CAPS caps = {};
        const bool capsOk = HidP_GetCaps(preparsed, &caps) == HIDP_STATUS_SUCCESS;
        HidD_FreePreparsedData(preparsed);
        if(!capsOk || caps.InputReportByteLength == 0)
            continue;

        const bool isBluetooth = caps.InputReportByteLength != 64;

        // Bluetooth controller starts in simple mode.
        // Enable extended report and read a few times.
        if(isBluetooth)
            enableExtendedBluetoothReport(handle);

        std::vector<BYTE> report(caps.InputReportByteLength);
        int percent = -1;
        for(int attempt = 0; attempt < (isBluetooth ? 5 : 1); ++attempt) {
            if(!readInputReport(handle, report))
                break;
            percent = DualSenseProtocol::parseBatteryPercent(report, isBluetooth);
            if(percent >= 0)
                break;
        }
        if(percent < 0)
            continue;

        result.append({attrs.ProductID == kDualSenseEdgeProductId ? QStringLiteral("DualSense Edge Controller")
                                                                    : QStringLiteral("DualSense Controller"),
                       percent, isBluetooth ? QStringLiteral("bluetooth") : QStringLiteral("usb"),
                       QStringLiteral("gamepad")});
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
    return result;
}
