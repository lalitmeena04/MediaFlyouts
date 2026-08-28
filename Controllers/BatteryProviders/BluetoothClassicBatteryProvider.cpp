#include "BluetoothClassicBatteryProvider.h"

#include <windows.h>

#include <cfgmgr32.h>
// devpkey.h only defines storage for its DEVPKEY_* GUIDs when INITGUID is set
#include <initguid.h>
#include <devpkey.h>

#include <vector>

namespace {

// Undocumented but stable PnP property Windows Settings itself uses to show battery
// level for classic-Bluetooth devices (0-100, DEVPROP_TYPE_BYTE).
constexpr DEVPROPKEY kBluetoothBatteryKey = {
    {0x104ea319, 0x6ee2, 0x4701, {0xbd, 0x47, 0x8d, 0xdb, 0xf4, 0x25, 0xbb, 0xe5}}, 2};

QString devNodeFriendlyName(DEVINST devInst) {
    DEVPROPTYPE type = 0;
    ULONG size = 0;
    CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_FriendlyName, &type, nullptr, &size, 0);
    if(size == 0)
        return {};

    std::vector<wchar_t> buf(size / sizeof(wchar_t));
    if(CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_FriendlyName, &type, reinterpret_cast<PBYTE>(buf.data()),
                                 &size, 0) != CR_SUCCESS)
        return {};
    return QString::fromWCharArray(buf.data());
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

} // namespace

QVector<BatteryDeviceInfo> BluetoothClassicBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    ULONG len = 0;
    if(CM_Get_Device_ID_List_SizeW(&len, L"Bluetooth", CM_GETIDLIST_FILTER_CLASS) != CR_SUCCESS || len < 2)
        return result;

    std::vector<wchar_t> ids(len);
    if(CM_Get_Device_ID_ListW(L"Bluetooth", ids.data(), len, CM_GETIDLIST_FILTER_CLASS) != CR_SUCCESS)
        return result;

    for(const wchar_t* id = ids.data(); *id; id += wcslen(id) + 1) {
        DEVINST devInst = 0;
        if(CM_Locate_DevNodeW(&devInst, const_cast<DEVINSTID_W>(id), CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
            continue;
        if(!devNodeIsActive(devInst))
            continue;

        DEVPROPTYPE type = 0;
        BYTE percent = 0;
        ULONG size = sizeof(percent);
        if(CM_Get_DevNode_PropertyW(devInst, &kBluetoothBatteryKey, &type, &percent, &size, 0) != CR_SUCCESS)
            continue;
        if(type != DEVPROP_TYPE_BYTE)
            continue;

        const QString name = devNodeFriendlyName(devInst);
        if(name.isEmpty())
            continue;

        result.append({name, static_cast<int>(percent), QStringLiteral("bluetooth")});
    }

    return result;
}
