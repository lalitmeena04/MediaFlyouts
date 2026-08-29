#include "HidCollections.h"

#include <setupapi.h>

std::vector<HidCollection> enumerateHidCollections(USHORT vendorId) {
    std::vector<HidCollection> collections;

    GUID hidGuid = {};
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO set = SetupDiGetClassDevsW(&hidGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(set == INVALID_HANDLE_VALUE)
        return collections;

    SP_DEVICE_INTERFACE_DATA iface = {};
    iface.cbSize = sizeof(iface);
    for(DWORD index = 0; SetupDiEnumDeviceInterfaces(set, nullptr, &hidGuid, index, &iface); ++index) {
        DWORD needed = 0;
        SetupDiGetDeviceInterfaceDetailW(set, &iface, nullptr, 0, &needed, nullptr);
        if(needed == 0)
            continue;

        std::vector<BYTE> buffer(needed);
        auto* detail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(buffer.data());
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        if(!SetupDiGetDeviceInterfaceDetailW(set, &iface, detail, needed, nullptr, nullptr))
            continue;

        HANDLE device = CreateFileW(detail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                    OPEN_EXISTING, 0, nullptr);
        if(device == INVALID_HANDLE_VALUE)
            continue;

        HIDD_ATTRIBUTES attrs = {};
        attrs.Size = sizeof(attrs);
        if(!HidD_GetAttributes(device, &attrs) || attrs.VendorID != vendorId) {
            CloseHandle(device);
            continue;
        }

        HidCollection collection;
        collection.path = QString::fromWCharArray(detail->DevicePath);
        collection.vendorId = attrs.VendorID;
        collection.productId = attrs.ProductID;

        PHIDP_PREPARSED_DATA preparsed = nullptr;
        if(HidD_GetPreparsedData(device, &preparsed)) {
            HIDP_CAPS caps = {};
            if(HidP_GetCaps(preparsed, &caps) == HIDP_STATUS_SUCCESS) {
                collection.usagePage = caps.UsagePage;
                collection.usage = caps.Usage;
                collection.inputLength = caps.InputReportByteLength;
                collection.outputLength = caps.OutputReportByteLength;
                collection.featureLength = caps.FeatureReportByteLength;
            }
            HidD_FreePreparsedData(preparsed);
        }

        wchar_t product[128] = {};
        if(HidD_GetProductString(device, product, sizeof(product)))
            collection.product = QString::fromWCharArray(product).trimmed();

        CloseHandle(device);
        collections.push_back(collection);
    }

    SetupDiDestroyDeviceInfoList(set);
    return collections;
}

QString categoryFromHidUsage(USAGE usagePage, USAGE usage) {
    if(usagePage != 0x01) // Generic Desktop
        return {};
    switch(usage) {
    case 0x02: return QStringLiteral("mouse");
    case 0x06: return QStringLiteral("keyboard");
    case 0x04: // joystick
    case 0x05: return QStringLiteral("gamepad");
    default: return {};
    }
}

QString categoryForProduct(const std::vector<HidCollection>& collections, USHORT productId) {
    bool hasMouse = false;
    bool hasGamepad = false;
    bool hasKeyboard = false;
    for(const HidCollection& collection : collections) {
        if(collection.productId != productId)
            continue;
        const QString category = categoryFromHidUsage(collection.usagePage, collection.usage);
        hasMouse = hasMouse || category == QLatin1String("mouse");
        hasGamepad = hasGamepad || category == QLatin1String("gamepad");
        hasKeyboard = hasKeyboard || category == QLatin1String("keyboard");
    }
    if(hasMouse)
        return QStringLiteral("mouse");
    if(hasGamepad)
        return QStringLiteral("gamepad");
    if(hasKeyboard)
        return QStringLiteral("keyboard");
    return {};
}
