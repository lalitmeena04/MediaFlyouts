#include "WiredPeripheralRules.h"

#include <QSet>
#include <QStringList>

namespace mf {

namespace {

bool containsInsensitive(const QStringList& list, const QString& value) {
    return list.contains(value.trimmed(), Qt::CaseInsensitive);
}

} // namespace

bool isStorageDeviceClass(const QString& deviceClass) {
    static const QStringList kStorage{
        QStringLiteral("USBSTOR"),     QStringLiteral("DiskDrive"),  QStringLiteral("Volume"),
        QStringLiteral("VolumeSnapshot"), QStringLiteral("CDROM"),   QStringLiteral("FloppyDisk"),
        QStringLiteral("SCSIAdapter"), QStringLiteral("TapeDrive"),  QStringLiteral("MTD"),
        // Phones and cameras exposed as MTP storage.
        QStringLiteral("WPD")};
    return containsInsensitive(kStorage, deviceClass);
}

bool isInfrastructureDeviceClass(const QString& deviceClass) {
    static const QStringList kInfrastructure{
        // Hubs, composite parents and host controllers all report class USB.
        QStringLiteral("USB"),      QStringLiteral("USBFN"),   QStringLiteral("System"),
        QStringLiteral("Computer"), QStringLiteral("HDC"),     QStringLiteral("Processor"),
        QStringLiteral("SoftwareDevice"), QStringLiteral("SoftwareComponent"),
        // The host radio itself. Its paired devices come from the Bluetooth providers.
        QStringLiteral("Bluetooth")};
    return containsInsensitive(kInfrastructure, deviceClass);
}

bool isGenericDeviceName(const QString& name) {
    static const QStringList kGeneric{QStringLiteral("USB Input Device"),
                                      QStringLiteral("USB Composite Device"),
                                      QStringLiteral("USB Audio Device"),
                                      QStringLiteral("Generic USB Hub"),
                                      QStringLiteral("USB Root Hub"),
                                      QStringLiteral("Unknown Device"),
                                      QStringLiteral("HID-compliant device"),
                                      // USB-C alt-mode descriptor. Not a device the user plugged in.
                                      QStringLiteral("Generic Billboard Device"),
                                      QStringLiteral("USB Attached SCSI (UAS) Mass Storage Device")};
    return containsInsensitive(kGeneric, name);
}

bool isWirelessReceiverName(const QString& name) {
    static const QStringList kMarkers{QStringLiteral("receiver"), QStringLiteral("dongle"),
                                      QStringLiteral("2.4g"), QStringLiteral("2.4 g"),
                                      QStringLiteral("unifying")};
    for(const QString& marker : kMarkers)
        if(name.contains(marker, Qt::CaseInsensitive))
            return true;
    return false;
}

bool isAnnounceableWiredDevice(const QString& deviceClass, const QString& name) {
    if(name.trimmed().isEmpty() || isGenericDeviceName(name) || isWirelessReceiverName(name))
        return false;
    return !isStorageDeviceClass(deviceClass) && !isInfrastructureDeviceClass(deviceClass);
}

QString categoryFromDeviceClass(const QString& deviceClass) {
    const QString normalized = deviceClass.trimmed();
    if(normalized.compare(QLatin1String("Keyboard"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("keyboard");
    if(normalized.compare(QLatin1String("Mouse"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("mouse");
    if(normalized.compare(QLatin1String("XnaComposite"), Qt::CaseInsensitive) == 0
       || normalized.compare(QLatin1String("XboxComposite"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("gamepad");
    if(normalized.compare(QLatin1String("Camera"), Qt::CaseInsensitive) == 0
       || normalized.compare(QLatin1String("Image"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("camera");
    if(normalized.compare(QLatin1String("Media"), Qt::CaseInsensitive) == 0
       || normalized.compare(QLatin1String("AudioEndpoint"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("headset");
    return {};
}

bool parseUsbVendorProduct(const QString& instanceId, quint16& vendorId, quint16& productId) {
    const int vendorAt = instanceId.indexOf(QLatin1String("VID_"), 0, Qt::CaseInsensitive);
    const int productAt = instanceId.indexOf(QLatin1String("PID_"), 0, Qt::CaseInsensitive);
    if(vendorAt < 0 || productAt < 0)
        return false;

    bool vendorOk = false;
    bool productOk = false;
    const uint vendor = instanceId.mid(vendorAt + 4, 4).toUInt(&vendorOk, 16);
    const uint product = instanceId.mid(productAt + 4, 4).toUInt(&productOk, 16);
    if(!vendorOk || !productOk)
        return false;

    vendorId = static_cast<quint16>(vendor);
    productId = static_cast<quint16>(product);
    return true;
}

QString mergedDeviceName(const QStringList& names) {
    QStringList cleaned;
    for(const QString& name : names) {
        const QString trimmed = name.trimmed();
        if(!trimmed.isEmpty() && !cleaned.contains(trimmed, Qt::CaseInsensitive))
            cleaned.append(trimmed);
    }
    if(cleaned.size() < 2)
        return cleaned.value(0);

    QString prefix = cleaned.first();
    for(const QString& name : cleaned) {
        int shared = 0;
        while(shared < prefix.size() && shared < name.size()
              && prefix.at(shared).toCaseFolded() == name.at(shared).toCaseFolded())
            ++shared;
        prefix.truncate(shared);
    }

    // Drop a half-finished word, unless the prefix is already a whole function name.
    if(!cleaned.contains(prefix, Qt::CaseInsensitive)) {
        const int lastSpace = prefix.lastIndexOf(QLatin1Char(' '));
        prefix = lastSpace > 0 ? prefix.left(lastSpace) : QString();
    }
    prefix = prefix.trimmed();

    // Too little in common to name the device. Keep the first function name.
    return prefix.size() >= 3 ? prefix : cleaned.first();
}

QString mergedCategory(const QStringList& categories) {
    static const QStringList kByPrecedence{QStringLiteral("camera"), QStringLiteral("gamepad"),
                                           QStringLiteral("keyboard"), QStringLiteral("mouse"),
                                           QStringLiteral("headset")};
    for(const QString& candidate : kByPrecedence)
        if(categories.contains(candidate, Qt::CaseInsensitive))
            return candidate;

    for(const QString& category : categories)
        if(!category.trimmed().isEmpty())
            return category.trimmed();
    return {};
}

} // namespace mf
