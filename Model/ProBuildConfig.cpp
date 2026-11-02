#include "ProBuildConfig.h"

#include <QTimeZone>

#include "Logging.h"

namespace mf {

namespace {
constexpr int kDefaultTrialDays = 15;
}

bool ProBuildConfig::purchaseConfigured() const {
    return !proAddOnStoreId.isEmpty();
}

ProBuildConfig parseProBuildConfig(const QString& proAddOnStoreId, const QString& cutoverUtc, const QString& trialDays,
                                   const QString& tipAddOnStoreIds) {
    ProBuildConfig c;
    c.proAddOnStoreId = proAddOnStoreId.trimmed();

    const QString cutoverText = cutoverUtc.trimmed();
    if(!cutoverText.isEmpty()) {
        QDateTime cutover = QDateTime::fromString(cutoverText, Qt::ISODate);
        if(!cutover.isValid()) {
            TRC_WARN("pro config: cutover '{}' is not ISO-8601, gating stays off", cutoverText.toStdString());
        } else {
            // A bare date-time with no zone is read as UTC, not local time.
            if(cutover.timeSpec() == Qt::LocalTime)
                cutover = QDateTime(cutover.date(), cutover.time(), QTimeZone::UTC);
            c.policy.cutover = cutover.toUTC();
        }
    }

    bool ok = false;
    const int days = trialDays.trimmed().toInt(&ok);
    if(ok && days >= 0) {
        c.policy.trialDays = days;
    } else {
        if(!trialDays.trimmed().isEmpty())
            TRC_WARN("pro config: trial days '{}' invalid, using {}", trialDays.toStdString(), kDefaultTrialDays);
        c.policy.trialDays = kDefaultTrialDays;
    }

    const QStringList rawTips = tipAddOnStoreIds.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for(const QString& id : rawTips) {
        const QString clean = id.trimmed();
        if(!clean.isEmpty() && !c.tipAddOnStoreIds.contains(clean))
            c.tipAddOnStoreIds << clean;
    }
    return c;
}

ProBuildConfig proBuildConfig() {
    static const ProBuildConfig config = [] {
        const ProBuildConfig c = parseProBuildConfig(
            QString::fromLatin1(MEDIAFLYOUTS_PRO_ADDON_STORE_ID), QString::fromLatin1(MEDIAFLYOUTS_STORE_CUTOVER_UTC),
            QString::fromLatin1(MEDIAFLYOUTS_TRIAL_DAYS), QString::fromLatin1(MEDIAFLYOUTS_TIP_ADDON_STORE_IDS));
        TRC_INFO("pro config: addOn='{}' cutover={} trialDays={} tips={}", c.proAddOnStoreId.toStdString(),
                 c.policy.cutover.isValid() ? c.policy.cutover.toString(Qt::ISODate).toStdString() : "none",
                 c.policy.trialDays, c.tipAddOnStoreIds.size());
        return c;
    }();
    return config;
}

} // namespace mf
