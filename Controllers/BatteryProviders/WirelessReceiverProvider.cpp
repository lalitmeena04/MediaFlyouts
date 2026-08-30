#include "WirelessReceiverProvider.h"

#include "HidCollections.h"

#include <QSet>

namespace {

const WirelessReceiverProfile kReceiverProfiles[] = {
    {0x046D, 0x0ADF, QStringLiteral("Logitech G435"), QStringLiteral("headset")},
    {0x3554, 0xFA09, QStringLiteral("AULA wireless keyboard"), QStringLiteral("keyboard")},
};

void appendReceivers(QVector<BatteryDeviceInfo>& result, QSet<QString>& seenNames, quint16 vendorId) {
    const std::vector<HidCollection> collections = enumerateHidCollections(vendorId);
    for(const HidCollection& collection : collections) {
        const WirelessReceiverProfile* profile = wirelessReceiverProfile(collection.vendorId, collection.productId);
        if(!profile || seenNames.contains(profile->name))
            continue;
        seenNames.insert(profile->name);
        result.append({profile->name, -1, QStringLiteral("wireless"), profile->category});
    }
}

} // namespace

const WirelessReceiverProfile* wirelessReceiverProfile(quint16 vendorId, quint16 productId) {
    for(const WirelessReceiverProfile& profile : kReceiverProfiles)
        if(profile.vendorId == vendorId && profile.productId == productId)
            return &profile;
    return nullptr;
}

QVector<BatteryDeviceInfo> WirelessReceiverProvider::poll() {
    QVector<BatteryDeviceInfo> result;
    QSet<QString> seenNames;
    for(const WirelessReceiverProfile& profile : kReceiverProfiles)
        appendReceivers(result, seenNames, profile.vendorId);
    return result;
}
