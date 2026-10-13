#pragma once

#include <QString>
#include <QStringList>

namespace mf {

bool isStorageDeviceClass(const QString& deviceClass);

bool isInfrastructureDeviceClass(const QString& deviceClass);

// Placeholder text Windows uses when a device reports no product string.
bool isGenericDeviceName(const QString& name);

bool isWirelessReceiverName(const QString& name);

bool isAnnounceableWiredDevice(const QString& deviceClass, const QString& name);

QString categoryFromDeviceClass(const QString& deviceClass);

bool parseUsbVendorProduct(const QString& instanceId, quint16& vendorId, quint16& productId);

QString mergedDeviceName(const QStringList& names);

// A webcam also exposes a microphone, so camera must outrank headset.
QString mergedCategory(const QStringList& categories);

} // namespace mf
