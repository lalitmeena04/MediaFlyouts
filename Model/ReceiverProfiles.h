#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

// Proprietary 2.4 GHz receivers that report no battery.
// These facts differ per device and per firmware revision, so they are data,
// not code. Users can override them without a rebuild.
namespace mf {

struct ReceiverProfile {
        quint16 vendorId = 0;
        quint16 productId = 0;
        QString name;
        QString category;
        // Same peripheral plugged in by cable. It enumerates under its own ids.
        // A dongle stays present when unused, so the cable wins. Zero means none.
        quint16 wiredVendorId = 0;
        quint16 wiredProductId = 0;
};

// Profiles shipped with the app. A starting point, not a device database.
QVector<ReceiverProfile> defaultReceiverProfiles();

// Read profiles from JSON. Skip malformed entries and describe each in errors.
QVector<ReceiverProfile> parseReceiverProfiles(const QByteArray& json, QStringList* errors = nullptr);

// Lay user profiles over the built-in ones. Equal vendor and product ids replace.
QVector<ReceiverProfile> mergeReceiverProfiles(const QVector<ReceiverProfile>& base,
                                               const QVector<ReceiverProfile>& overrides);

// File the user drops next to the settings ini to add their own receivers.
QString receiverProfilesFileName();

} // namespace mf
