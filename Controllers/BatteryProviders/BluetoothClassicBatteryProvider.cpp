#include "BluetoothClassicBatteryProvider.h"

#include "BatteryUtils.h"
#include "DevNodeHelpers.h"

#include <windows.h>

#include <cfgmgr32.h>
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
        if(!bluetoothAddressConnected(devNodeStringProperty(devInst, kBluetoothDeviceAddressKey)))
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
        if(!bluetoothAddressConnected(devNodeStringProperty(devInst, kBluetoothDeviceAddressKey)))
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
