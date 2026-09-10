#include "ReceiverProfiles.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace mf {

namespace {

// Ids may be written as "046D", "0x046D" or as a plain number.
bool readId(const QJsonValue& value, quint16& out) {
    if(value.isUndefined() || value.isNull()) {
        out = 0;
        return true;
    }
    if(value.isDouble()) {
        const int number = value.toInt(-1);
        if(number < 0 || number > 0xFFFF)
            return false;
        out = static_cast<quint16>(number);
        return true;
    }
    if(!value.isString())
        return false;

    QString text = value.toString().trimmed();
    if(text.startsWith(QLatin1String("0x"), Qt::CaseInsensitive))
        text = text.mid(2);
    if(text.isEmpty() || text.size() > 4)
        return false;

    bool ok = false;
    const uint parsed = text.toUInt(&ok, 16);
    if(!ok || parsed > 0xFFFF)
        return false;
    out = static_cast<quint16>(parsed);
    return true;
}

void reportError(QStringList* errors, const QString& message) {
    if(errors)
        errors->append(message);
}

} // namespace

QVector<ReceiverProfile> defaultReceiverProfiles() {
    return {{0x046D, 0x0ADF, QStringLiteral("Logitech G435"), QStringLiteral("headset")},
            {0x3554, 0xFA09, QStringLiteral("AULA wireless keyboard"), QStringLiteral("keyboard"), 0x258A, 0x010C}};
}

QString receiverProfilesFileName() {
    return QStringLiteral("receivers.json");
}

QVector<ReceiverProfile> parseReceiverProfiles(const QByteArray& json, QStringList* errors) {
    QVector<ReceiverProfile> profiles;
    if(json.trimmed().isEmpty())
        return profiles;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if(parseError.error != QJsonParseError::NoError) {
        reportError(errors, QStringLiteral("invalid JSON: %1").arg(parseError.errorString()));
        return profiles;
    }

    // Accept a bare array or an object holding a "receivers" array.
    QJsonArray entries;
    if(document.isArray())
        entries = document.array();
    else if(document.isObject())
        entries = document.object().value(QStringLiteral("receivers")).toArray();
    if(entries.isEmpty()) {
        reportError(errors, QStringLiteral("no receiver entries found"));
        return profiles;
    }

    int index = 0;
    for(const QJsonValue& value : entries) {
        const int at = index++;
        if(!value.isObject()) {
            reportError(errors, QStringLiteral("entry %1 is not an object").arg(at));
            continue;
        }
        const QJsonObject entry = value.toObject();

        ReceiverProfile profile;
        if(!readId(entry.value(QStringLiteral("vid")), profile.vendorId)
           || !readId(entry.value(QStringLiteral("pid")), profile.productId)) {
            reportError(errors, QStringLiteral("entry %1 has an unreadable vid or pid").arg(at));
            continue;
        }
        if(profile.vendorId == 0 || profile.productId == 0) {
            reportError(errors, QStringLiteral("entry %1 is missing vid or pid").arg(at));
            continue;
        }
        if(!readId(entry.value(QStringLiteral("wiredVid")), profile.wiredVendorId)
           || !readId(entry.value(QStringLiteral("wiredPid")), profile.wiredProductId)) {
            reportError(errors, QStringLiteral("entry %1 has an unreadable wiredVid or wiredPid").arg(at));
            continue;
        }
        // A half-declared twin would match every device at id zero.
        if((profile.wiredVendorId == 0) != (profile.wiredProductId == 0)) {
            reportError(errors, QStringLiteral("entry %1 needs both wiredVid and wiredPid").arg(at));
            continue;
        }

        profile.name = entry.value(QStringLiteral("name")).toString().trimmed();
        if(profile.name.isEmpty()) {
            reportError(errors, QStringLiteral("entry %1 is missing a name").arg(at));
            continue;
        }
        profile.category = entry.value(QStringLiteral("category")).toString().trimmed();
        profiles.append(profile);
    }
    return profiles;
}

QVector<ReceiverProfile> mergeReceiverProfiles(const QVector<ReceiverProfile>& base,
                                               const QVector<ReceiverProfile>& overrides) {
    QVector<ReceiverProfile> merged = base;
    for(const ReceiverProfile& profile : overrides) {
        bool replaced = false;
        for(ReceiverProfile& existing : merged) {
            if(existing.vendorId != profile.vendorId || existing.productId != profile.productId)
                continue;
            existing = profile;
            replaced = true;
            break;
        }
        if(!replaced)
            merged.append(profile);
    }
    return merged;
}

} // namespace mf
