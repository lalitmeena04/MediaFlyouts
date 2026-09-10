#include "WirelessReceiverProvider.h"

#include "HidCollections.h"
#include "Logging.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSettings>

namespace {

// Same folder as the settings ini, so config and profiles sit together.
QString receiverProfilesPath() {
    const QSettings store(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                          QStringLiteral("MediaFlyouts"));
    return QDir(QFileInfo(store.fileName()).absolutePath()).filePath(mf::receiverProfilesFileName());
}

// Read once. Editing receivers.json takes effect on the next launch.
QVector<mf::ReceiverProfile> loadProfiles() {
    const QVector<mf::ReceiverProfile> defaults = mf::defaultReceiverProfiles();

    QFile file(receiverProfilesPath());
    if(!file.exists())
        return defaults;
    if(!file.open(QIODevice::ReadOnly)) {
        TRC_WARN("cannot read {}", file.fileName().toStdString());
        return defaults;
    }

    QStringList errors;
    const QVector<mf::ReceiverProfile> userProfiles = mf::parseReceiverProfiles(file.readAll(), &errors);
    for(const QString& error : errors)
        TRC_WARN("receivers.json: {}", error.toStdString());
    TRC_INFO("loaded {} user receiver profiles", userProfiles.size());
    return mf::mergeReceiverProfiles(defaults, userProfiles);
}

const QVector<mf::ReceiverProfile>& receiverProfiles() {
    static const QVector<mf::ReceiverProfile> profiles = loadProfiles();
    return profiles;
}

// The dongle proves only its own presence, never that the peripheral is paired.
bool wiredTwinPresent(const mf::ReceiverProfile& profile) {
    if(profile.wiredVendorId == 0)
        return false;
    const std::vector<HidCollection> collections = enumerateHidCollections(profile.wiredVendorId);
    for(const HidCollection& collection : collections)
        if(collection.productId == profile.wiredProductId)
            return true;
    return false;
}

void appendReceivers(QVector<BatteryDeviceInfo>& result, QSet<QString>& seenNames, quint16 vendorId) {
    const std::vector<HidCollection> collections = enumerateHidCollections(vendorId);
    for(const HidCollection& collection : collections) {
        const mf::ReceiverProfile* profile = wirelessReceiverProfile(collection.vendorId, collection.productId);
        if(!profile || seenNames.contains(profile->name))
            continue;
        if(wiredTwinPresent(*profile))
            continue;
        seenNames.insert(profile->name);
        result.append({profile->name, -1, QStringLiteral("wireless"), profile->category});
    }
}

} // namespace

const mf::ReceiverProfile* wirelessReceiverProfile(quint16 vendorId, quint16 productId) {
    for(const mf::ReceiverProfile& profile : receiverProfiles())
        if(profile.vendorId == vendorId && profile.productId == productId)
            return &profile;
    return nullptr;
}

QVector<BatteryDeviceInfo> WirelessReceiverProvider::poll() {
    QVector<BatteryDeviceInfo> result;
    QSet<QString> seenNames;
    QSet<quint16> polledVendors;
    for(const mf::ReceiverProfile& profile : receiverProfiles()) {
        if(polledVendors.contains(profile.vendorId))
            continue;
        polledVendors.insert(profile.vendorId);
        appendReceivers(result, seenNames, profile.vendorId);
    }
    return result;
}
