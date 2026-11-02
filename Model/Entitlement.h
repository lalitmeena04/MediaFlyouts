#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>

#include "InstallSource.h"

namespace mf {

enum class Tier {
    Free,
    Trial,
    Pro,
};

enum class ProFeature {
    Visualizer,
    PerAppMixer,
    PeripheralBattery,
    NowPlayingStyles,
    TaskbarWidgetOptions,
};

enum class EntitlementSource {
    Unlocked,
    AddOn,
    Grandfathered,
    StoreTrial,
    LocalTrial,
    Cached,
    Expired,
    Override,
};

// `available` false = the Store gave no answer.
struct StoreFacts {
    bool available = false;
    bool addOnLicensed = false;
    QDateTime acquired;
};

struct EntitlementCache {
    bool valid = false;
    Tier tier = Tier::Free;
    QDateTime trialEnd;
    QDateTime lastSeen;
};

// Invalid cutover = gating off.
struct EntitlementPolicy {
    bool gatingEnabled = true;
    QDateTime cutover;
    int trialDays = 15;
};

struct EntitlementInputs {
    InstallKind install = InstallKind::Unknown;
    StoreFacts store;
    EntitlementCache cache;
    EntitlementPolicy policy;
    QDateTime now;
};

struct EntitlementState {
    Tier tier = Tier::Free;
    EntitlementSource source = EntitlementSource::Unlocked;
    QDateTime trialEnd;
    // max(now, cache.lastSeen). A clock set back cannot extend a trial.
    QDateTime effectiveNow;
    bool clockRolledBack = false;

    bool operator==(const EntitlementState& other) const;
    bool operator!=(const EntitlementState& other) const;
};

// Pure. Decision table in dev-notes/PRO-PAYWALL-PLAN.md.
EntitlementState resolveEntitlement(const EntitlementInputs& in);

EntitlementCache cacheFrom(const EntitlementState& state);

bool isUnlocked(Tier tier);
bool isUnlocked(ProFeature feature, Tier tier);

// Rounded up. 0 when `end` is not after `now`.
int trialDaysLeft(const QDateTime& now, const QDateTime& end);

QString tierId(Tier tier);
QString sourceId(EntitlementSource source);
QString featureId(ProFeature feature);
bool featureFromId(const QString& id, ProFeature* feature);
QStringList allFeatureIds();

QString describeEntitlement(const EntitlementState& state);

} // namespace mf