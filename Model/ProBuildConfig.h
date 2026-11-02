#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>

#include "Entitlement.h"

// Fallbacks for tools that build without the CMake definitions.
#ifndef MEDIAFLYOUTS_PRO_ADDON_STORE_ID
#define MEDIAFLYOUTS_PRO_ADDON_STORE_ID ""
#endif
#ifndef MEDIAFLYOUTS_STORE_CUTOVER_UTC
#define MEDIAFLYOUTS_STORE_CUTOVER_UTC ""
#endif
#ifndef MEDIAFLYOUTS_TRIAL_DAYS
#define MEDIAFLYOUTS_TRIAL_DAYS "15"
#endif
#ifndef MEDIAFLYOUTS_TIP_ADDON_STORE_IDS
#define MEDIAFLYOUTS_TIP_ADDON_STORE_IDS ""
#endif

namespace mf {

struct ProBuildConfig {
    QString proAddOnStoreId;
    QStringList tipAddOnStoreIds;
    EntitlementPolicy policy;

    bool purchaseConfigured() const;
};

// Pure. Bad cutover = gating off. Bad trial days = 15.
ProBuildConfig parseProBuildConfig(const QString& proAddOnStoreId, const QString& cutoverUtc, const QString& trialDays,
                                   const QString& tipAddOnStoreIds);

ProBuildConfig proBuildConfig();

} // namespace mf
