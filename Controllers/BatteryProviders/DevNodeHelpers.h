#pragma once

#include <windows.h>

#include <cfgmgr32.h>

#include <functional>
#include <QString>

using DevNodeVisitor = std::function<void(DEVINST devInst, const QString& instanceId)>;
using DeviceIdVisitor = std::function<void(const QString& instanceId)>;

bool forEachDevNode(const wchar_t* enumerator, const DevNodeVisitor& visitor);
void forEachDeviceIdList(const wchar_t* ids, const DeviceIdVisitor& visitor);

QString devNodeStringProperty(DEVINST devInst, const DEVPROPKEY& key);

QString devNodeContainerId(DEVINST devInst);

bool devNodeIsActive(DEVINST devInst);

bool parseBluetoothAddressHex(const QString& hex, quint64& out);

// DN_STARTED means paired, not connected. Ask the Bluetooth API instead.
bool bluetoothAddressConnected(quint64 address);
bool bluetoothAddressConnected(const QString& addressHex);
