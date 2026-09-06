#include "BluetoothClassicBatteryProvider.h"

#include "BatteryUtils.h"

#include <windows.h>

#include <bluetoothapis.h>
#include <cfgmgr32.h>
#include <combaseapi.h>
// Header devpkey.h defines DEVPKEY GUID storage with INITGUID.
#include <initguid.h>
#include <devpkey.h>

#include <QSet>

#include <vector>

namespace {

// Stable PnP property for classic Bluetooth battery.
// Value is 0-100, DEVPROP_TYPE_BYTE. BTHENUM sub-node varies by device.
constexpr DEVPROPKEY kBluetoothBatteryKey = {
    {0x104ea319, 0x6ee2, 0x4701, {0xbd, 0x47, 0x8d, 0xdb, 0xf4, 0x25, 0xbb, 0xe5}}, 2};

// Runtime property schema defines this key. SDK headers do not.
// Value is classic Bluetooth MAC address as hex string.
constexpr DEVPROPKEY kBluetoothDeviceAddressKey = {
    {0x2bd67d8b, 0x8beb, 0x48d5, {0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a}}, 1};

// Runtime property schema defines this key. SDK headers do not.
// Value is 24-bit Bluetooth Class of Device.
constexpr DEVPROPKEY kBluetoothClassOfDeviceKey = {
    {0x2bd67d8b, 0x8beb, 0x48d5, {0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a}}, 4};

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

// Device PnP container id. Dedupes profile sub-nodes.
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

// Report active devices only. Matches Get-PnpDevice "OK" filter.
// Avoid stale battery values for disconnected devices.
bool devNodeIsActive(DEVINST devInst) {
    ULONG status = 0, problem = 0;
    if(CM_Get_DevNode_Status(&status, &problem, devInst, 0) != CR_SUCCESS)
        return false;
    return (status & DN_STARTED) && !(status & DN_HAS_PROBLEM);
}

// DN_STARTED means paired or present. It does not mean connected.
// Ask Bluetooth API for live connection state.
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

    // Enumerate BTHENUM. Bluetooth setup class can miss profile sub-nodes.
    ULONG len = 0;
    if(CM_Get_Device_ID_List_SizeW(&len, L"BTHENUM", CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS || len < 2)
        return result;

    std::vector<wchar_t> ids(len);
    if(CM_Get_Device_ID_ListW(L"BTHENUM", ids.data(), len, CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS)
        return result;

    // Device can expose battery on more than one profile sub-node. Keep first hit.
    QSet<QString> seenContainers;
    QSet<QString> seenNames;

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
        seenNames.insert(name);

        DEVPROPTYPE codType = 0;
        quint32 classOfDevice = 0;
        ULONG codSize = sizeof(classOfDevice);
        QString category;
        if(CM_Get_DevNode_PropertyW(devInst, &kBluetoothClassOfDeviceKey, &codType, reinterpret_cast<PBYTE>(&classOfDevice),
                                    &codSize, 0) == CR_SUCCESS
           && codType == DEVPROP_TYPE_UINT32)
            category = BatteryUtils::categoryFromClassOfDevice(classOfDevice);

        result.append({name, static_cast<int>(percent), QStringLiteral("bluetooth"), category});
    }

    // Second pass reports connected devices with unknown battery.
    // Connect/disconnect events still surface.
    for(const wchar_t* id = ids.data(); *id; id += wcslen(id) + 1) {
        DEVINST devInst = 0;
        if(CM_Locate_DevNodeW(&devInst, const_cast<DEVINSTID_W>(id), CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
            continue;
        if(!devNodeIsActive(devInst))
            continue;
        if(!isBluetoothAddressConnected(devNodeStringProperty(devInst, kBluetoothDeviceAddressKey)))
            continue;

        const QString containerId = devNodeContainerId(devInst);
        if(!containerId.isEmpty()) {
            if(seenContainers.contains(containerId))
                continue;
            seenContainers.insert(containerId);
        }

        const QString name =
            BatteryUtils::cleanBluetoothProfileSuffix(devNodeStringProperty(devInst, DEVPKEY_Device_FriendlyName));
        if(name.isEmpty() || seenNames.contains(name))
            continue;
        seenNames.insert(name);

        DEVPROPTYPE codType = 0;
        quint32 classOfDevice = 0;
        ULONG codSize = sizeof(classOfDevice);
        QString category;
        if(CM_Get_DevNode_PropertyW(devInst, &kBluetoothClassOfDeviceKey, &codType, reinterpret_cast<PBYTE>(&classOfDevice),
                                    &codSize, 0) == CR_SUCCESS
           && codType == DEVPROP_TYPE_UINT32)
            category = BatteryUtils::categoryFromClassOfDevice(classOfDevice);

        result.append({name, -1, QStringLiteral("bluetooth"), category});
    }

    return result;
}
