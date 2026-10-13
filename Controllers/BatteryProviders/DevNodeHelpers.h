#pragma once

#include <windows.h>

#include <cfgmgr32.h>

#include <QString>

QString devNodeStringProperty(DEVINST devInst, const DEVPROPKEY& key);

QString devNodeContainerId(DEVINST devInst);

bool devNodeIsActive(DEVINST devInst);

bool parseBluetoothAddressHex(const QString& hex, quint64& out);

// DN_STARTED means paired, not connected. Ask the Bluetooth API instead.
bool bluetoothAddressConnected(quint64 address);
bool bluetoothAddressConnected(const QString& addressHex);
