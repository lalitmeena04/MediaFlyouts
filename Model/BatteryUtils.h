#pragma once

#include <QString>
#include <QtGlobal>

namespace BatteryUtils {

QString cleanBluetoothProfileSuffix(const QString& rawName);

int milliwattHoursToPercent(long long remainingMwh, long long fullChargeMwh);

QString categoryFromClassOfDevice(quint32 classOfDevice);

} // namespace BatteryUtils
