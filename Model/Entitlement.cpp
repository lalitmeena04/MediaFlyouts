#include "Entitlement.h"

namespace mf {

namespace {

constexpr qint64 kMsPerDay = 24 * 60 * 60 * 1000;

EntitlementState grant(EntitlementState s, Tier tier, EntitlementSource source) {
    s.tier = tier;
    s.source = source;
    return s;
}

// Trial while `now` is before trialEnd. Expired after that.
EntitlementState trialOrExpired(EntitlementState s, EntitlementSource source) {
    if(s.effectiveNow < s.trialEnd)
        return grant(s, Tier::Trial, source);
    return grant(s, Tier::Free, EntitlementSource::Expired);
}

} // namespace

bool EntitlementState::operator==(const EntitlementState& o) const {
    return tier == o.tier && source == o.source && trialEnd == o.trialEnd && effectiveNow == o.effectiveNow
           && clockRolledBack == o.clockRolledBack;
}

bool EntitlementState::operator!=(const EntitlementState& o) const {
    return !(*this == o);
}

EntitlementState resolveEntitlement(const EntitlementInputs& in) {
    EntitlementState s;
    s.effectiveNow = in.now;
    const QDateTime& lastSeen = in.cache.lastSeen;
    if(in.cache.valid && lastSeen.isValid() && (!in.now.isValid() || in.now < lastSeen)) {
        // The clock went backwards. Time only moves forward here.
        s.effectiveNow = lastSeen;
        s.clockRolledBack = in.now.isValid();
    }

    // Fail open. A build without a cutover or a Store-less install never locks anything.
    if(!in.policy.gatingEnabled || !in.policy.cutover.isValid() || in.install != InstallKind::Store
       || !s.effectiveNow.isValid())
        return grant(s, Tier::Pro, EntitlementSource::Unlocked);

    if(in.store.available) {
        if(in.store.addOnLicensed)
            return grant(s, Tier::Pro, EntitlementSource::AddOn);
        if(in.store.acquired.isValid()) {
            if(in.store.acquired < in.policy.cutover)
                return grant(s, Tier::Pro, EntitlementSource::Grandfathered);
            s.trialEnd = in.store.acquired.addDays(in.policy.trialDays);
            return trialOrExpired(s, EntitlementSource::StoreTrial);
        }
        // The Store answered without an acquisition date. The cache decides.
    }

    if(in.cache.valid) {
        if(in.cache.tier == Tier::Pro)
            return grant(s, Tier::Pro, EntitlementSource::Cached);
        if(in.cache.trialEnd.isValid()) {
            s.trialEnd = in.cache.trialEnd;
            return trialOrExpired(s, EntitlementSource::Cached);
        }
    }

    // First run with no Store answer. The trial starts now.
    s.trialEnd = s.effectiveNow.addDays(in.policy.trialDays);
    return trialOrExpired(s, EntitlementSource::LocalTrial);
}

EntitlementCache cacheFrom(const EntitlementState& state) {
    EntitlementCache c;
    c.valid = true;
    c.tier = state.tier;
    c.trialEnd = state.trialEnd;
    c.lastSeen = state.effectiveNow;
    return c;
}

bool isUnlocked(Tier tier) {
    return tier != Tier::Free;
}

bool isUnlocked(ProFeature, Tier tier) {
    // Every Pro feature follows the same tier today. The feature stays in the
    // signature so a per-feature rule can be added without touching callers.
    return isUnlocked(tier);
}

int trialDaysLeft(const QDateTime& now, const QDateTime& end) {
    if(!now.isValid() || !end.isValid())
        return 0;
    const qint64 ms = now.msecsTo(end);
    if(ms <= 0)
        return 0;
    return static_cast<int>((ms + kMsPerDay - 1) / kMsPerDay);
}

QString tierId(Tier tier) {
    switch(tier) {
        case Tier::Free: return QStringLiteral("free");
        case Tier::Trial: return QStringLiteral("trial");
        case Tier::Pro: return QStringLiteral("pro");
    }
    return QStringLiteral("free");
}

QString sourceId(EntitlementSource source) {
    switch(source) {
        case EntitlementSource::Unlocked: return QStringLiteral("unlocked");
        case EntitlementSource::AddOn: return QStringLiteral("addOn");
        case EntitlementSource::Grandfathered: return QStringLiteral("grandfathered");
        case EntitlementSource::StoreTrial: return QStringLiteral("storeTrial");
        case EntitlementSource::LocalTrial: return QStringLiteral("localTrial");
        case EntitlementSource::Cached: return QStringLiteral("cached");
        case EntitlementSource::Expired: return QStringLiteral("expired");
    }
    return QStringLiteral("unlocked");
}

namespace {

struct FeatureName {
    ProFeature feature;
    const char* id;
};

constexpr FeatureName kFeatureNames[] = {
    {ProFeature::Visualizer, "visualizer"},
    {ProFeature::PerAppMixer, "perAppMixer"},
    {ProFeature::PeripheralBattery, "peripheralBattery"},
    {ProFeature::NowPlayingStyles, "nowPlayingStyles"},
    {ProFeature::TaskbarWidgetOptions, "taskbarWidgetOptions"},
};

} // namespace

QString featureId(ProFeature feature) {
    for(const FeatureName& n : kFeatureNames)
        if(n.feature == feature)
            return QString::fromLatin1(n.id);
    return {};
}

bool featureFromId(const QString& id, ProFeature* feature) {
    for(const FeatureName& n : kFeatureNames) {
        if(id == QLatin1String(n.id)) {
            if(feature)
                *feature = n.feature;
            return true;
        }
    }
    return false;
}

QStringList allFeatureIds() {
    QStringList ids;
    for(const FeatureName& n : kFeatureNames)
        ids << QString::fromLatin1(n.id);
    return ids;
}

QString describeEntitlement(const EntitlementState& s) {
    return QStringLiteral("tier=%1 source=%2 trialEnd=%3 daysLeft=%4 clockRolledBack=%5")
        .arg(tierId(s.tier), sourceId(s.source),
             s.trialEnd.isValid() ? s.trialEnd.toUTC().toString(Qt::ISODate) : QStringLiteral("none"))
        .arg(trialDaysLeft(s.effectiveNow, s.trialEnd))
        .arg(s.clockRolledBack ? QStringLiteral("true") : QStringLiteral("false"));
}

} // namespace mf
