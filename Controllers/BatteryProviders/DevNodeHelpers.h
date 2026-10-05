#pragma once

#include <windows.h>

#include <cfgmgr32.h>

#include <QString>

// CM_* device-node helpers shared by the PnP-based battery providers.
// One copy keeps every provider reading properties the same way.

// String property. Empty when missing, wrong type, or on error.
// Trims stray whitespace some drivers append.
QString devNodeStringProperty(DEVINST devInst, const DEVPROPKEY& key);

// PnP container id as GUID text. Groups the function nodes of one device.
QString devNodeContainerId(DEVINST devInst);

// True when started and problem-free. Matches Get-PnpDevice "OK".
bool devNodeIsActive(DEVINST devInst);

// Parse a 12-digit hex Bluetooth address. False on bad input.
bool parseBluetoothAddressHex(const QString& hex, quint64& out);

// DN_STARTED means paired, not connected. Ask the Bluetooth API instead.
bool bluetoothAddressConnected(quint64 address);
bool bluetoothAddressConnected(const QString& addressHex);
