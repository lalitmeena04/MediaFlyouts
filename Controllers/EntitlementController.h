#pragma once

#include <functional>
#include <memory>
#include <optional>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "Entitlement.h"
#include "EntitlementStore.h"
#include "InstallSource.h"
#include "ProBuildConfig.h"
#include "StoreGateway.h"

// The `entitlement` QML context property. Works without a gateway: the fallbacks then apply.
class EntitlementController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString tier READ tierId NOTIFY changed)
        Q_PROPERTY(QString source READ sourceId NOTIFY changed)
        Q_PROPERTY(bool unlocked READ unlocked NOTIFY changed)
        Q_PROPERTY(bool isPro READ isPro NOTIFY changed)
        Q_PROPERTY(bool isTrial READ isTrial NOTIFY changed)
        Q_PROPERTY(int trialDaysLeft READ trialDaysLeft NOTIFY changed)
        Q_PROPERTY(bool gatingEnabled READ gatingEnabled CONSTANT)
        Q_PROPERTY(bool storeAvailable READ storeAvailable NOTIFY changed)
        Q_PROPERTY(bool canPurchase READ canPurchase NOTIFY changed)
        Q_PROPERTY(QString price READ price NOTIFY changed)
        Q_PROPERTY(bool purchasing READ purchasing NOTIFY changed)
        Q_PROPERTY(bool refreshing READ refreshing NOTIFY changed)
        Q_PROPERTY(QString lastPurchaseStatus READ lastPurchaseStatus NOTIFY changed)
        Q_PROPERTY(QStringList tipAddOnStoreIds READ tipAddOnStoreIds CONSTANT)
        Q_PROPERTY(QString proAddOnStoreId READ proAddOnStoreId CONSTANT)
        Q_PROPERTY(QStringList featureIds READ featureIds CONSTANT)

    public:
        using Clock = std::function<QDateTime()>;

        EntitlementController(mf::InstallKind install, const mf::ProBuildConfig& config,
                              std::unique_ptr<mf::EntitlementStore> store, QObject* parent = nullptr);
        ~EntitlementController() override;

        // Not owned. Set before start().
        void setGateway(StoreGateway* gateway);
        void setClock(Clock clock);
        // start() also reads MEDIAFLYOUTS_ENTITLEMENT in debug builds.
        void setOverride(std::optional<mf::Tier> tier);

        void start();

        const mf::EntitlementState& state() const;
        const StoreSnapshot& snapshot() const;
        mf::InstallKind installKind() const;

        QString tierId() const;
        QString sourceId() const;
        bool unlocked() const;
        bool isPro() const;
        bool isTrial() const;
        int trialDaysLeft() const;
        bool gatingEnabled() const;
        bool storeAvailable() const;
        bool canPurchase() const;
        QString price() const;
        bool purchasing() const;
        bool refreshing() const;
        QString lastPurchaseStatus() const;
        QStringList tipAddOnStoreIds() const;
        QString proAddOnStoreId() const;
        QStringList featureIds() const;

        // Unknown ids read as unlocked. A typo must not lock a feature.
        Q_INVOKABLE bool isUnlocked(const QString& featureId) const;
        // `ownerWindow` is the QML Window that owns the Store dialog.
        Q_INVOKABLE void purchase(QObject* ownerWindow = nullptr);
        Q_INVOKABLE void purchaseAddOn(const QString& storeId, QObject* ownerWindow = nullptr);
        Q_INVOKABLE void refresh();
        void reevaluate();

        static std::optional<mf::Tier> parseOverride(const QString& value);
        static QString overrideEnvironmentVariable();

    signals:
        void changed();
        void purchaseFinished(bool ok, const QString& statusId, const QString& storeId);

    private:
        void resolve();
        void scheduleReevaluation();
        void onSnapshot(const StoreSnapshot& snapshot);
        void onGatewayPurchaseFinished(const QString& storeId, bool ok, const QString& statusId);
        bool beginPurchase(const QString& storeId, QObject* ownerWindow);

        mf::InstallKind m_install;
        mf::ProBuildConfig m_config;
        std::unique_ptr<mf::EntitlementStore> m_store;
        StoreGateway* m_gateway = nullptr;
        Clock m_clock;
        std::optional<mf::Tier> m_override;

        mf::EntitlementCache m_cache;
        StoreSnapshot m_snapshot;
        mf::EntitlementState m_state;
        QTimer m_reevaluate;
        bool m_started = false;
        bool m_refreshing = false;
        bool m_purchasing = false;
        QString m_lastPurchaseStatus;
};
