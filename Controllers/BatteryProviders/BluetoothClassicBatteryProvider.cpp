#include "BluetoothClassicBatteryProvider.h"

#include "BatteryUtils.h"

#include <windows.h>

#include <bluetoothapis.h>
#include <cfgmgr32.h>
#include <combaseapi.h>
// devpkey.h only defines storage for its DEVPKEY_* GUIDs when INITGUID is set
#include <initguid.h>
#include <devpkey.h>

#include <QSet>

#include <vector>

namespace {

// Undocumented but stable PnP property Windows Settings itself uses to show battery
// level for classic-Bluetooth devices (0-100, DEVPROP_TYPE_BYTE). Lives on whichever
// BTHENUM sub-node actually tracks it for that device (e.g. HID devices report it on
// their main node, audio headsets report it on their "Hands-Free AG" profile node).
constexpr DEVPROPKEY kBluetoothBatteryKey = {
    {0x104ea319, 0x6ee2, 0x4701, {0xbd, 0x47, 0x8d, 0xdb, 0xf4, 0x25, 0xbb, 0xe5}}, 2};

// Also not declared in any SDK header (resolved only via the OS's runtime property
// schema); the classic Bluetooth device's MAC address as a hex string.
constexpr DEVPROPKEY kBluetoothDeviceAddressKey = {
    {0x2bd67d8b, 0x8beb, 0x48d5, {0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a}}, 1};

// Same undeclared-property situation; the 24-bit Bluetooth Class of Device field
// (major/minor device class), used to identify the actual device category.
constexpr DEVPROPKEY kBluetoothClassOfDeviceKey = {
    {0x2bd67d8b, 0x8beb, 0x48d5, {0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a}}, 4};

// Decodes the Bluetooth Class of Device field (see Bluetooth Assigned Numbers, "Baseband")
// into a rough device category. Falls back to empty for classes we don't map explicitly.
QString categoryFromClassOfDevice(quint32 cod) {
    const quint32 majorClass = (cod >> 8) & 0x1F;
    const quint32 minorClass = (cod >> 2) & 0x3F;

    if(majorClass == 0x04) // Audio/Video
        return QStringLiteral("headset");
    if(majorClass == 0x05) { // Peripheral (HID)
        const quint32 pointerKeyboardBits = (minorClass >> 4) & 0x3;
        if(pointerKeyboardBits == 0x1)
            return QStringLiteral("keyboard");
        if(pointerKeyboardBits == 0x2)
            return QStringLiteral("mouse");
        if(pointerKeyboardBits == 0x3)
            return QStringLiteral("keyboard"); // combo keyboard/pointing device

        const quint32 subtype = minorClass & 0xF;
        if(subtype == 0x1 || subtype == 0x2) // joystick / gamepad
            return QStringLiteral("gamepad");
    }
    return {};
}

QString devNodeStringProperty(DEVINST devInst, const DEVPROPKEY& key) {
    DEVPROPTYPE type = 0;
    ULONG size = 0;
    CM_Get_DevNode_PropertyW(devInst, &key, &type, nullptr, &size, 0);
    if(size == 0)
        return {};

    std::vector<wchar_t> buf(size / sizeof(wchar_t));
    if(CM_Get_DevNode_PropertyW(devInst, &key, &type, reinterpret_cast<PBYTE>(buf.data()), &size, 0) != CR_SUCCESS)
        return {};
    return QString::fromWCharArray(buf.data());
}

// The device's PnP container id, used to dedupe multiple profile sub-nodes reporting
// battery for the same physical device.
QString devNodeContainerId(DEVINST devInst) {
    DEVPROPTYPE type = 0;
    GUID containerId = {};
    ULONG size = sizeof(containerId);
    if(CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_ContainerId, &type, reinterpret_cast<PBYTE>(&containerId),
                                 &size, 0) != CR_SUCCESS
       || type != DEVPROP_TYPE_GUID)
        return {};

    wchar_t buf[64] = {};
    StringFromGUID2(containerId, buf, static_cast<int>(std::size(buf)));
    return QString::fromWCharArray(buf);
}

// Only report devices that are actually present/connected right now, matching
// Get-PnpDevice's "OK" status filter (avoids showing a stale battery % for a device
// that's been off for days).
bool devNodeIsActive(DEVINST devInst) {
    ULONG status = 0, problem = 0;
    if(CM_Get_DevNode_Status(&status, &problem, devInst, 0) != CR_SUCCESS)
        return false;
    return (status & DN_STARTED) && !(status & DN_HAS_PROBLEM);
}

// DN_STARTED only means the device node is paired/present, not actively connected -
// Windows keeps paired Bluetooth nodes started for quick reconnect even when the radio
// link is down. Ask the Bluetooth API directly for the live connection state instead.
bool isBluetoothAddressConnected(const QString& addressHex) {
    bool ok = false;
    const quint64 address = addressHex.toULongLong(&ok, 16);
    if(!ok)
        return false;

    BLUETOOTH_DEVICE_INFO info = {};
    info.dwSize = sizeof(info);
    info.Address.ullLong = address;
    if(BluetoothGetDeviceInfo(nullptr, &info) != ERROR_SUCCESS)
        return false;
    return info.fConnected != FALSE;
}

} // namespace

QVector<BatteryDeviceInfo> BluetoothClassicBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    // enumerate by PnP enumerator (BTHENUM) rather than the "Bluetooth" setup class:
    // per-profile sub-nodes (e.g. Hands-Free AG, where audio-headset battery lives)
    // aren't reliably included under the class filter, but all of them share BTHENUM.
    ULONG len = 0;
    if(CM_Get_Device_ID_List_SizeW(&len, L"BTHENUM", CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS || len < 2)
        return result;

    std::vector<wchar_t> ids(len);
    if(CM_Get_Device_ID_ListW(L"BTHENUM", ids.data(), len, CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS)
        return result;

    // a physical device can expose the battery property on more than one profile
    // sub-node; keep only the first hit per device (by container id)
    QSet<QString> seenContainers;

    for(const wchar_t* id = ids.data(); *id; id += wcslen(id) + 1) {
        DEVINST devInst = 0;
        if(CM_Locate_DevNodeW(&devInst, const_cast<DEVINSTID_W>(id), CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
            continue;
        if(!devNodeIsActive(devInst))
            continue;
        if(!isBluetoothAddressConnected(devNodeStringProperty(devInst, kBluetoothDeviceAddressKey)))
            continue;

        DEVPROPTYPE type = 0;
        BYTE percent = 0;
        ULONG size = sizeof(percent);
        if(CM_Get_DevNode_PropertyW(devInst, &kBluetoothBatteryKey, &type, &percent, &size, 0) != CR_SUCCESS)
            continue;
        if(type != DEVPROP_TYPE_BYTE)
            continue;

        const QString containerId = devNodeContainerId(devInst);
        if(!containerId.isEmpty()) {
            if(seenContainers.contains(containerId))
                continue;
            seenContainers.insert(containerId);
        }

        const QString name =
            BatteryUtils::cleanBluetoothProfileSuffix(devNodeStringProperty(devInst, DEVPKEY_Device_FriendlyName));
        if(name.isEmpty())
            continue;

        DEVPROPTYPE codType = 0;
        quint32 classOfDevice = 0;
        ULONG codSize = sizeof(classOfDevice);
        QString category;
        if(CM_Get_DevNode_PropertyW(devInst, &kBluetoothClassOfDeviceKey, &codType, reinterpret_cast<PBYTE>(&classOfDevice),
                                    &codSize, 0) == CR_SUCCESS
           && codType == DEVPROP_TYPE_UINT32)
            category = categoryFromClassOfDevice(classOfDevice);

        result.append({name, static_cast<int>(percent), QStringLiteral("bluetooth"), category});
    }

    return result;
}
