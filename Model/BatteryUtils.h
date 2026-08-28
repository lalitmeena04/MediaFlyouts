#pragma once

#include <QString>
#include <QtGlobal>

// Small pure-logic helpers shared by battery providers, factored out for unit testing
// without needing real Bluetooth/game-controller hardware.
namespace BatteryUtils {

// BTHENUM sub-nodes are named after their Bluetooth profile (e.g. "Foo Hands-Free AG",
// "Foo Avrcp Transport"); strips that off so the UI shows the plain product name.
QString cleanBluetoothProfileSuffix(const QString& rawName);

// Converts a Windows.Gaming.Input BatteryReport's raw milliwatt-hour capacities into a
// 0-100 percentage. Returns -1 if the values are missing/invalid (e.g. full <= 0).
int milliwattHoursToPercent(long long remainingMwh, long long fullChargeMwh);

// Decodes the Bluetooth Class of Device field (see Bluetooth Assigned Numbers, "Baseband")
// into a rough device category: "headset", "keyboard", "mouse" or "gamepad". Returns an
// empty string for classes we don't map explicitly.
QString categoryFromClassOfDevice(quint32 classOfDevice);

} // namespace BatteryUtils
