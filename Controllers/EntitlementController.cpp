#include "EntitlementController.h"

#include <algorithm>

#include <QWindow>

#include "Logging.h"

using mf::EntitlementSource;
using mf::Tier;

namespace {

constexpr qint64 kMsPerDay = 24 * 60 * 60 * 1000;
constexpr char kOverrideEnv[] = "MEDIAFLYOUTS_ENTITLEMENT";

// effectiveNow moves on every call and is left out.
bool visibleStateEquals(const mf::EntitlementState& a, const mf::EntitlementState& b) {
    return a.tier == b.tier && a.source == b.source && a.trialEnd == b.trialEnd;
}

} // namespace

EntitlementController::EntitlementController(mf::InstallKind install, const mf::ProBuildConfig& config,
                                             std::unique_ptr<mf::EntitlementStore> store, QObject* parent)
    : QObject(parent), m_install(install), m_config(config), m_store(std::move(store)),
      m_clock([] { return QDateTime::currentDateTimeUtc(); }) {
    m_reevaluate.setSingleShot(true);
    connect(&m_reevaluate, &QTimer::timeout, this, &EntitlementController::reevaluate);
}

EntitlementController::~EntitlementController() = default;

void EntitlementController::setGateway(StoreGateway* gateway) {
    if(m_gateway)
        disconnect(m_gateway, nullptr, this, nullptr);
    m_gateway = gateway;
    if(!m_gateway)
        return;
    connect(m_gateway, &StoreGateway::snapshotReady, this, &EntitlementController::onSnapshot);
    connect(m_gateway, &StoreGateway::purchaseFinished, this, &EntitlementController::onGatewayPurchaseFinished);
    connect(m_gateway, &StoreGateway::licensesChanged, this, &EntitlementController::refresh);
}

void EntitlementController::setClock(Clock clock) {
    if(clock)
        m_clock = std::move(clock);
}

void EntitlementController::setOverride(std::optional<Tier> tier) {
    m_override = tier;
    if(m_started)
        resolve();
}

void EntitlementController::start() {
    m_started = true;
    if(m_store)
        m_cache = m_store->load();
#ifndef QT_NO_DEBUG
    if(!m_override) {
        const QString env = qEnvironmentVariable(kOverrideEnv);
        if(!env.isEmpty()) {
            m_override = parseOverride(env);
            if(m_override)
                TRC_WARN("entitlement: {} override active ({})", kOverrideEnv, env.toStdString());
            else
                TRC_WARN("entitlement: {} value '{}' ignored", kOverrideEnv, env.toStdString());
        }
    }
#endif
    TRC_INFO("entitlement: install={} gating={} cutover={} trialDays={} gateway={} cache={}",
             mf::installKindId(m_install).toStdString(), gatingEnabled(),
             m_config.policy.cutover.isValid() ? m_config.policy.cutover.toString(Qt::ISODate).toStdString() : "none",
             m_config.policy.trialDays, m_gateway ? (m_gateway->supported() ? "supported" : "unsupported") : "none",
             m_cache.valid ? mf::tierId(m_cache.tier).toStdString() : "none");
    resolve();
    refresh();
}

const mf::EntitlementState& EntitlementController::state() const {
    return m_state;
}

const StoreSnapshot& EntitlementController::snapshot() const {
    return m_snapshot;
}

mf::InstallKind EntitlementController::installKind() const {
    return m_install;
}

QString EntitlementController::tierId() const {
    return mf::tierId(m_state.tier);
}

QString EntitlementController::sourceId() const {
    return mf::sourceId(m_state.source);
}

bool EntitlementController::unlocked() const {
    return mf::isUnlocked(m_state.tier);
}

bool EntitlementController::isPro() const {
    return m_state.tier == Tier::Pro;
}

bool EntitlementController::isTrial() const {
    return m_state.tier == Tier::Trial;
}

int EntitlementController::trialDaysLeft() const {
    return mf::trialDaysLeft(m_state.effectiveNow, m_state.trialEnd);
}

bool EntitlementController::gatingEnabled() const {
    return m_config.policy.gatingEnabled && m_config.policy.cutover.isValid();
}

bool EntitlementController::storeAvailable() const {
    return m_gateway && m_gateway->supported();
}

bool EntitlementController::canPurchase() const {
    return m_install == mf::InstallKind::Store && storeAvailable() && m_config.purchaseConfigured() && !isPro()
           && !m_purchasing;
}

QString EntitlementController::price() const {
    return m_snapshot.proPrice;
}

bool EntitlementController::purchasing() const {
    return m_purchasing;
}

bool EntitlementController::refreshing() const {
    return m_refreshing;
}

QString EntitlementController::lastPurchaseStatus() const {
    return m_lastPurchaseStatus;
}

QStringList EntitlementController::tipAddOnStoreIds() const {
    return m_config.tipAddOnStoreIds;
}

QStringList EntitlementController::featureIds() const {
    return mf::allFeatureIds();
}

bool EntitlementController::isUnlocked(const QString& featureId) const {
    mf::ProFeature feature;
    if(!mf::featureFromId(featureId, &feature)) {
        TRC_WARN("entitlement: unknown feature id '{}', treated as unlocked", featureId.toStdString());
        return true;
    }
    return mf::isUnlocked(feature, m_state.tier);
}

void EntitlementController::purchase(QObject* ownerWindow) {
    if(isPro()) {
        emit purchaseFinished(true, mf::purchase::kAlreadyPurchased, m_config.proAddOnStoreId);
        return;
    }
    beginPurchase(m_config.proAddOnStoreId, ownerWindow);
}

void EntitlementController::purchaseAddOn(const QString& storeId, QObject* ownerWindow) {
    beginPurchase(storeId, ownerWindow);
}

bool EntitlementController::beginPurchase(const QString& storeId, QObject* ownerWindow) {
    if(storeId.trimmed().isEmpty()) {
        TRC_WARN("entitlement: purchase requested without an add-on id");
        emit purchaseFinished(false, mf::purchase::kNotConfigured, storeId);
        return false;
    }
    if(!storeAvailable()) {
        TRC_WARN("entitlement: purchase of {} requested without a Store", storeId.toStdString());
        emit purchaseFinished(false, mf::purchase::kUnavailable, storeId);
        return false;
    }
    if(m_purchasing) {
        emit purchaseFinished(false, mf::purchase::kBusy, storeId);
        return false;
    }
    m_gateway->setWindow(qobject_cast<QWindow*>(ownerWindow));
    m_purchasing = true;
    m_lastPurchaseStatus.clear();
    emit changed();
    TRC_INFO("entitlement: purchasing {}", storeId.toStdString());
    m_gateway->purchase(storeId.trimmed());
    return true;
}

void EntitlementController::refresh() {
    if(!storeAvailable())
        return;
    m_refreshing = true;
    emit changed();
    m_gateway->refresh();
}

void EntitlementController::reevaluate() {
    resolve();
}

std::optional<Tier> EntitlementController::parseOverride(const QString& value) {
    const QString v = value.trimmed().toLower();
    for(Tier t : {Tier::Free, Tier::Trial, Tier::Pro})
        if(mf::tierId(t) == v)
            return t;
    return std::nullopt;
}

QString EntitlementController::overrideEnvironmentVariable() {
    return QString::fromLatin1(kOverrideEnv);
}

void EntitlementController::onSnapshot(const StoreSnapshot& snapshot) {
    m_refreshing = false;
    m_snapshot = snapshot;
    resolve();
    // Price or availability may change without the tier moving.
    emit changed();
}

void EntitlementController::onGatewayPurchaseFinished(const QString& storeId, bool ok, const QString& statusId) {
    m_purchasing = false;
    m_lastPurchaseStatus = statusId;
    TRC_INFO("entitlement: purchase {} finished ok={} status={}", storeId.toStdString(), ok, statusId.toStdString());
    emit changed();
    emit purchaseFinished(ok, statusId, storeId);
    if(ok)
        refresh();
}

void EntitlementController::resolve() {
    mf::EntitlementInputs in;
    in.install = m_install;
    in.store.available = m_snapshot.available;
    in.store.addOnLicensed = m_snapshot.addOnLicensed;
    in.store.acquired = m_snapshot.acquired;
    in.cache = m_cache;
    in.policy = m_config.policy;
    in.now = m_clock();

    mf::EntitlementState next = mf::resolveEntitlement(in);
    if(m_override) {
        next.tier = *m_override;
        next.source = EntitlementSource::Override;
        if(next.tier == Tier::Trial && !next.trialEnd.isValid())
            next.trialEnd = next.effectiveNow.addDays(m_config.policy.trialDays);
    }

    const bool moved = !visibleStateEquals(next, m_state);
    m_state = next;

    // Unlocked and override states say nothing about this user.
    const bool cacheable =
        m_state.source != EntitlementSource::Unlocked && m_state.source != EntitlementSource::Override;
    if(cacheable) {
        m_cache = mf::cacheFrom(m_state);
        if(m_store)
            m_store->save(m_cache);
    }

    if(moved)
        TRC_INFO("entitlement: {}", mf::describeEntitlement(m_state).toStdString());
    else
        TRC_DEBUG("entitlement: unchanged, {}", mf::describeEntitlement(m_state).toStdString());

    scheduleReevaluation();
    if(moved)
        emit changed();
}

void EntitlementController::scheduleReevaluation() {
    // Wake at the trial end, else once a day.
    qint64 ms = kMsPerDay;
    if(m_state.tier == Tier::Trial && m_state.trialEnd.isValid() && m_state.effectiveNow.isValid())
        ms = std::clamp<qint64>(m_state.effectiveNow.msecsTo(m_state.trialEnd) + 1000, 1000, kMsPerDay);
    m_reevaluate.start(static_cast<int>(ms));
}
