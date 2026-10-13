#include "WiredPeripheralProvider.h"

#include "DevNodeHelpers.h"
#include "WiredPeripheralRules.h"
#include "WirelessReceiverProvider.h"

#include <windows.h>

#include <cfgmgr32.h>
// Header devpkey.h defines DEVPKEY GUID storage with INITGUID.
#include <initguid.h>
#include <devpkey.h>

#include <QHash>
#include <QStringList>

#include <vector>

namespace {

// The device's own product string reads better than the driver description.
QString devNodeDisplayName(DEVINST devInst) {
    const QString busReported = devNodeStringProperty(devInst, DEVPKEY_Device_BusReportedDeviceDesc);
    if(!busReported.isEmpty())
        return busReported;
    const QString friendly = devNodeStringProperty(devInst, DEVPKEY_Device_FriendlyName);
    if(!friendly.isEmpty())
        return friendly;
    return devNodeStringProperty(devInst, DEVPKEY_Device_DeviceDesc);
}

struct WiredContainer {
        QStringList names;
        QStringList categories;
};

// A dongle is not the peripheral. WirelessReceiverProvider reports its paired device.
bool isKnownReceiver(const QString& instanceId) {
    quint16 vendorId = 0;
    quint16 productId = 0;
    if(!mf::parseUsbVendorProduct(instanceId, vendorId, productId))
        return false;
    return wirelessReceiverProfile(vendorId, productId) != nullptr;
}

} // namespace

QVector<BatteryDeviceInfo> WiredPeripheralProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    ULONG len = 0;
    if(CM_Get_Device_ID_List_SizeW(&len, L"USB", CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS || len < 2)
        return result;

    std::vector<wchar_t> ids(len);
    if(CM_Get_Device_ID_ListW(L"USB", ids.data(), len, CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS)
        return result;

    // Keep insertion order so the list stays stable between polls.
    QStringList order;
    QHash<QString, WiredContainer> containers;

    for(const wchar_t* id = ids.data(); *id; id += wcslen(id) + 1) {
        const QString instanceId = QString::fromWCharArray(id);
        if(isKnownReceiver(instanceId))
            continue;

        DEVINST devInst = 0;
        if(CM_Locate_DevNodeW(&devInst, const_cast<DEVINSTID_W>(id), CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
            continue;
        if(!devNodeIsActive(devInst))
            continue;

        const QString deviceClass = devNodeStringProperty(devInst, DEVPKEY_Device_Class);
        const QString name = devNodeDisplayName(devInst);
        if(!mf::isAnnounceableWiredDevice(deviceClass, name))
            continue;

        // Nodes with no container id cannot be grouped. Treat the name as the group.
        const QString containerId = devNodeContainerId(devInst);
        const QString key = containerId.isEmpty() ? name : containerId;
        if(!containers.contains(key))
            order.append(key);

        WiredContainer& container = containers[key];
        container.names.append(name);
        container.categories.append(mf::categoryFromDeviceClass(deviceClass));
    }

    for(const QString& key : order) {
        const WiredContainer& container = containers.value(key);
        BatteryDeviceInfo info;
        info.name = mf::mergedDeviceName(container.names);
        if(info.name.isEmpty())
            continue;
        info.connectionKind = QStringLiteral("usb");
        info.category = mf::mergedCategory(container.categories);
        result.append(info);
    }

    return result;
}
