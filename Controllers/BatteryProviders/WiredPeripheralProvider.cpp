#include "WiredPeripheralProvider.h"

#include "DevNodeHelpers.h"
#include "WiredPeripheralRules.h"
#include "JabraBatteryProvider.h"
#include "JabraGnpProtocol.h"
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
// A Jabra GNP device reports its own battery through JabraBatteryProvider.
bool isKnownReceiver(const QString& instanceId) {
    quint16 vendorId = 0;
    quint16 productId = 0;
    if(!mf::parseUsbVendorProduct(instanceId, vendorId, productId))
        return false;
    if(wirelessReceiverProfile(vendorId, productId) != nullptr)
        return true;
    return vendorId == gnp::kVendorId && jabraGnpProductPresent(productId);
}

} // namespace

QVector<BatteryDeviceInfo> WiredPeripheralProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    // Keep insertion order so the list stays stable between polls.
    QStringList order;
    QHash<QString, WiredContainer> containers;

    forEachDevNode(L"USB", [&](DEVINST devInst, const QString& instanceId) {
        if(isKnownReceiver(instanceId))
            return;
        if(!devNodeIsActive(devInst))
            return;

        const QString deviceClass = devNodeStringProperty(devInst, DEVPKEY_Device_Class);
        const QString name = devNodeDisplayName(devInst);
        if(!mf::isAnnounceableWiredDevice(deviceClass, name))
            return;

        // Nodes with no container id cannot be grouped. Treat the name as the group.
        const QString containerId = devNodeContainerId(devInst);
        const QString key = containerId.isEmpty() ? name : containerId;
        if(!containers.contains(key))
            order.append(key);

        WiredContainer& container = containers[key];
        container.names.append(name);
        container.categories.append(mf::categoryFromDeviceClass(deviceClass));
    });

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
