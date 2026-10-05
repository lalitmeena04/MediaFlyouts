#include "DevNodeHelpers.h"

#include <bluetoothapis.h>
#include <combaseapi.h>
// Header devpkey.h defines DEVPKEY GUID storage with INITGUID.
#include <initguid.h>
#include <devpkey.h>

#include <iterator>
#include <vector>

QString devNodeStringProperty(DEVINST devInst, const DEVPROPKEY& key) {
    DEVPROPTYPE type = 0;
    ULONG size = 0;
    CM_Get_DevNode_PropertyW(devInst, &key, &type, nullptr, &size, 0);
    if(size == 0)
        return {};

    std::vector<wchar_t> buf(size / sizeof(wchar_t));
    if(CM_Get_DevNode_PropertyW(devInst, &key, &type, reinterpret_cast<PBYTE>(buf.data()), &size, 0) != CR_SUCCESS
       || type != DEVPROP_TYPE_STRING)
        return {};
    return QString::fromWCharArray(buf.data()).trimmed();
}

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

bool devNodeIsActive(DEVINST devInst) {
    ULONG status = 0, problem = 0;
    if(CM_Get_DevNode_Status(&status, &problem, devInst, 0) != CR_SUCCESS)
        return false;
    return (status & DN_STARTED) && !(status & DN_HAS_PROBLEM);
}

bool parseBluetoothAddressHex(const QString& hex, quint64& out) {
    const QString trimmed = hex.trimmed();
    if(trimmed.isEmpty() || trimmed.size() > 12)
        return false;
    bool ok = false;
    const quint64 value = trimmed.toULongLong(&ok, 16);
    if(!ok)
        return false;
    out = value;
    return true;
}

bool bluetoothAddressConnected(quint64 address) {
    BLUETOOTH_DEVICE_INFO info = {};
    info.dwSize = sizeof(info);
    info.Address.ullLong = address;
    if(BluetoothGetDeviceInfo(nullptr, &info) != ERROR_SUCCESS)
        return false;
    return info.fConnected != FALSE;
}

bool bluetoothAddressConnected(const QString& addressHex) {
    quint64 address = 0;
    if(!parseBluetoothAddressHex(addressHex, address))
        return false;
    return bluetoothAddressConnected(address);
}
