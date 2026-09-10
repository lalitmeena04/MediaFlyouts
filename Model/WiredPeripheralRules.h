#pragma once

#include <QString>
#include <QStringList>

// Decide which wired devices count as peripherals.
// Keep rules free of Windows and Qt GUI types so tests need no hardware.
namespace mf {

// Setup classes that carry user data. Storage is never announced or listed.
bool isStorageDeviceClass(const QString& deviceClass);

// Hubs, composite parents, controllers and other plumbing.
// The user plugged in one device, not the tree Windows builds under it.
bool isInfrastructureDeviceClass(const QString& deviceClass);

// Placeholder text Windows uses when a device reports no product string.
bool isGenericDeviceName(const QString& name);

// A 2.4 GHz dongle is not the peripheral. Its paired device is reported as wireless.
bool isWirelessReceiverName(const QString& name);

// True when a wired device is worth announcing and listing.
bool isAnnounceableWiredDevice(const QString& deviceClass, const QString& name);

// Rough device category from the setup class. Empty when unknown.
QString categoryFromDeviceClass(const QString& deviceClass);

// Read VID and PID out of a PnP instance id such as USB\VID_046D&PID_0ADF\....
bool parseUsbVendorProduct(const QString& instanceId, quint16& vendorId, quint16& productId);

// One product name for the several function names of a single physical device.
// "Owl Lite Camera" plus "Owl Lite Audio" becomes "Owl Lite".
QString mergedDeviceName(const QStringList& names);

// Pick the category that identifies a merged device best.
// A webcam also exposes a microphone, so camera must outrank headset.
QString mergedCategory(const QStringList& categories);

} // namespace mf
