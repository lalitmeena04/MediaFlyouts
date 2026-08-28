#pragma once

#include <QString>

// Small pure-logic helpers shared by battery providers, factored out for unit testing
// without needing real Bluetooth/game-controller hardware.
namespace BatteryUtils {

// BTHENUM sub-nodes are named after their Bluetooth profile (e.g. "Foo Hands-Free AG",
// "Foo Avrcp Transport"); strips that off so the UI shows the plain product name.
QString cleanBluetoothProfileSuffix(const QString& rawName);

// Converts a Windows.Gaming.Input BatteryReport's raw milliwatt-hour capacities into a
// 0-100 percentage. Returns -1 if the values are missing/invalid (e.g. full <= 0).
int milliwattHoursToPercent(long long remainingMwh, long long fullChargeMwh);

} // namespace BatteryUtils
