#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>

#include "InstallSource.h"

namespace mf {

// What the user may use. Trial and Pro unlock the same features.
enum class Tier {
    Free,
    Trial,
    Pro,
};

// Features behind the Pro add-on.
enum class ProFeature {
    Visualizer,
    PerAppMixer,
    PeripheralBattery,
    NowPlayingStyles,
    TaskbarWidgetOptions,
};

// Why the tier was granted. Shown in logs and the upgrade card.
enum class EntitlementSource {
    Unlocked,      // non-Store build, gating off, or misconfigured build
    AddOn,         // Pro add-on licensed
    Grandfathered, // app acquired before the cutover, when it was paid
    StoreTrial,    // trial anchored to the Store acquisition date
    LocalTrial,    // trial anchored to the first run on this machine
    Cached,        // Store unreachable, last known state reused
    Expired,       // trial ran out
};

// What the Store answered. `available` false means no answer (offline, no account, API error).
struct StoreFacts {
    bool available = false;
    bool addOnLicensed = false;
    QDateTime acquired; // invalid when unknown
};

// Last resolved state. Lets the app keep working when the Store does not answer.
struct EntitlementCache {
    bool valid = false;
    Tier tier = Tier::Free;
    QDateTime trialEnd; // invalid when no trial was ever started
    QDateTime lastSeen; // highest clock value seen so far
};

struct EntitlementPolicy {
    bool gatingEnabled = true;
    QDateTime cutover; // invalid = not configured, everything unlocked
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
    QDateTime trialEnd;    // valid for Trial and Expired
    QDateTime effectiveNow; // `now`, or the cached lastSeen when the clock went backwards
    bool clockRolledBack = false;

    bool operator==(const EntitlementState& other) const;
    bool operator!=(const EntitlementState& other) const;
};

// Pure. Decides the tier from the inputs. See dev-notes/PRO-PAYWALL-PLAN.md for the table.
EntitlementState resolveEntitlement(const EntitlementInputs& in);

// Cache to persist after a resolution. Keeps lastSeen monotonic.
EntitlementCache cacheFrom(const EntitlementState& state);

bool isUnlocked(Tier tier);
bool isUnlocked(ProFeature feature, Tier tier);

// Whole days left, rounded up. 0 when `end` is not after `now` or invalid.
int trialDaysLeft(const QDateTime& now, const QDateTime& end);

QString tierId(Tier tier);
QString sourceId(EntitlementSource source);
QString featureId(ProFeature feature);
// False when `id` is not a known feature. `feature` is untouched then.
bool featureFromId(const QString& id, ProFeature* feature);
QStringList allFeatureIds();

// One line for logs: tier, source, trial end, clock state.
QString describeEntitlement(const EntitlementState& state);

} // namespace mf
