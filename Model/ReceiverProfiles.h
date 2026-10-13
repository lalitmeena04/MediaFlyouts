#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace mf {

struct ReceiverProfile {
        quint16 vendorId = 0;
        quint16 productId = 0;
        QString name;
        QString category;
        // A dongle stays present when unused, so the cable wins. Zero means none.
        quint16 wiredVendorId = 0;
        quint16 wiredProductId = 0;
};

QVector<ReceiverProfile> defaultReceiverProfiles();

QVector<ReceiverProfile> parseReceiverProfiles(const QByteArray& json, QStringList* errors = nullptr);

QVector<ReceiverProfile> mergeReceiverProfiles(const QVector<ReceiverProfile>& base,
                                               const QVector<ReceiverProfile>& overrides);

QString receiverProfilesFileName();

} // namespace mf
