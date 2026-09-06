#pragma once

#include <QString>
#include <QtGlobal>

// Pure logic helpers for battery providers.
// Unit tests do not need real Bluetooth or gamepad hardware.
namespace BatteryUtils {

// BTHENUM sub-nodes use Bluetooth profile names.
// Strip profile suffix for plain product name.
QString cleanBluetoothProfileSuffix(const QString& rawName);

// Convert BatteryReport milliwatt-hour values to 0-100 percent.
// Return -1 for missing or invalid values.
int milliwattHoursToPercent(long long remainingMwh, long long fullChargeMwh);

// Decode Bluetooth Class of Device to rough device category.
// Return empty string for unmapped classes.
QString categoryFromClassOfDevice(quint32 classOfDevice);

} // namespace BatteryUtils
