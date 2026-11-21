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

// Value is 0-100, DEVPROP_TYPE_BYTE. BTHENUM sub-node varies by device.
constexpr DEVPROPKEY kBluetoothBatteryKey = {
    {0x104ea319, 0x6ee2, 0x4701, {0xbd, 0x47, 0x8d, 0xdb, 0xf4, 0x25, 0xbb, 0xe5}}, 2};

// Runtime property schema defines this key. SDK headers do not.
// Value is classic Bluetooth MAC address as hex string.
constexpr DEVPROPKEY kBluetoothDeviceAddressKey = {
    {0x2bd67d8b, 0x8beb, 0x48d5, {0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a}}, 1};

// Value is 24-bit Bluetooth Class of Device.
constexpr DEVPROPKEY kBluetoothClassOfDeviceKey = {
    {0x2bd67d8b, 0x8beb, 0x48d5, {0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a}}, 4};

} // namespace

QVector<BatteryDeviceInfo> BluetoothClassicBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    // Device can expose battery on more than one profile sub-node. Keep first hit.
    QSet<QString> seenContainers;
    QSet<QString> seenNames;

    // Bluetooth setup class can miss profile sub-nodes, so enumerate BTHENUM.
    forEachDevNode(L"BTHENUM", [&](DEVINST devInst, const QString&) {
        if(!devNodeIsActive(devInst))
            return;
        if(!bluetoothAddressConnected(devNodeStringProperty(devInst, kBluetoothDeviceAddressKey)))
            return;

        DEVPROPTYPE type = 0;
        BYTE percent = 0;
        ULONG size = sizeof(percent);
        if(CM_Get_DevNode_PropertyW(devInst, &kBluetoothBatteryKey, &type, &percent, &size, 0) != CR_SUCCESS)
            return;
        if(type != DEVPROP_TYPE_BYTE)
            return;

        const QString containerId = devNodeContainerId(devInst);
        if(!containerId.isEmpty()) {
            if(seenContainers.contains(containerId))
                return;
            seenContainers.insert(containerId);
        }

        const QString name =
            BatteryUtils::cleanBluetoothProfileSuffix(devNodeStringProperty(devInst, DEVPKEY_Device_FriendlyName));
        if(name.isEmpty())
            return;
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
    });

    forEachDevNode(L"BTHENUM", [&](DEVINST devInst, const QString&) {
        if(!devNodeIsActive(devInst))
            return;
        if(!bluetoothAddressConnected(devNodeStringProperty(devInst, kBluetoothDeviceAddressKey)))
            return;

        const QString containerId = devNodeContainerId(devInst);
        if(!containerId.isEmpty()) {
            if(seenContainers.contains(containerId))
                return;
            seenContainers.insert(containerId);
        }

        const QString name =
            BatteryUtils::cleanBluetoothProfileSuffix(devNodeStringProperty(devInst, DEVPKEY_Device_FriendlyName));
        if(name.isEmpty() || seenNames.contains(name))
            return;
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
    });

    return result;
}
