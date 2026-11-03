#include "StoreLicensing.h"

#include <chrono>
#include <vector>

#include <QDateTime>
#include <QTimeZone>
#include <QWindow>

#include <windows.h>

#include <shobjidl.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include "InstallSource.h"
#include "Logging.h"

using winrt::Windows::Foundation::AsyncStatus;
using winrt::Windows::Foundation::IAsyncOperation;
using winrt::Windows::Services::Store::StoreAppLicense;
using winrt::Windows::Services::Store::StoreConsumableResult;
using winrt::Windows::Services::Store::StoreContext;
using winrt::Windows::Services::Store::StoreLicense;
using winrt::Windows::Services::Store::StoreProduct;
using winrt::Windows::Services::Store::StoreProductQueryResult;
using winrt::Windows::Services::Store::StoreProductResult;
using winrt::Windows::Services::Store::StorePurchaseResult;
using winrt::Windows::Services::Store::StorePurchaseStatus;
using winrt::Windows::Services::Store::StoreSku;

namespace {

QString toQString(const winrt::hstring& s) {
    return QString::fromWCharArray(s.c_str(), static_cast<int>(s.size()));
}

QDateTime toQDateTime(const winrt::Windows::Foundation::DateTime& dt) {
    // Zero means "not set" in the Store data.
    if(dt.time_since_epoch().count() == 0)
        return {};
    const auto sys = winrt::clock::to_sys(dt);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(sys.time_since_epoch()).count();
    return QDateTime::fromMSecsSinceEpoch(ms, QTimeZone::UTC);
}

winrt::Windows::Foundation::Collections::IIterable<winrt::hstring> kinds(std::initializer_list<const wchar_t*> names) {
    std::vector<winrt::hstring> list;
    for(const wchar_t* n : names)
        list.emplace_back(n);
    return winrt::single_threaded_vector<winrt::hstring>(std::move(list));
}

QString hresultText(const winrt::hresult_error& e) {
    return QStringLiteral("hresult:0x%1").arg(static_cast<unsigned long>(e.code()), 8, 16, QLatin1Char('0'));
}

} // namespace

StoreLicensing::StoreLicensing(const QString& proAddOnStoreId, const QStringList& consumableStoreIds, QObject* parent)
    : StoreGateway(parent), m_proAddOnStoreId(proAddOnStoreId.trimmed()) {
    for(const QString& id : consumableStoreIds)
        if(!id.trimmed().isEmpty())
            m_consumableStoreIds << id.trimmed();
    TRC_INFO("store: packaged={} addOn='{}' consumables={}", mf::isPackagedProcess(),
             m_proAddOnStoreId.toStdString(), m_consumableStoreIds.size());
}

StoreLicensing::~StoreLicensing() {
    m_licenseRevoker.revoke();
    // The worker captures `this`. It must finish first.
    if(m_worker && m_worker->isRunning())
        m_worker->wait();
}

bool StoreLicensing::supported() const {
    return mf::isPackagedProcess() && !m_contextFailed;
}

void StoreLicensing::setWindow(QWindow* window) {
    m_hwnd = window ? reinterpret_cast<void*>(window->winId()) : nullptr;
}

QString StoreLicensing::proAddOnStoreId() const {
    return m_proAddOnStoreId;
}

QStringList StoreLicensing::consumableStoreIds() const {
    return m_consumableStoreIds;
}

QString StoreLicensing::statusIdFor(StorePurchaseStatus status, bool* ok) {
    switch(status) {
        case StorePurchaseStatus::Succeeded:
            *ok = true;
            return mf::purchase::kSucceeded;
        case StorePurchaseStatus::AlreadyPurchased:
            *ok = true;
            return mf::purchase::kAlreadyPurchased;
        case StorePurchaseStatus::NotPurchased:
            *ok = false;
            return mf::purchase::kNotPurchased;
        case StorePurchaseStatus::NetworkError:
            *ok = false;
            return mf::purchase::kNetworkError;
        case StorePurchaseStatus::ServerError: break;
    }
    *ok = false;
    return mf::purchase::kServerError;
}

bool StoreLicensing::licenseKeyMatches(const QString& key, const QString& storeId) {
    if(storeId.isEmpty() || key.isEmpty())
        return false;
    if(key.compare(storeId, Qt::CaseInsensitive) == 0)
        return true;
    return key.startsWith(storeId + QLatin1Char('/'), Qt::CaseInsensitive);
}

StoreContext StoreLicensing::context() {
    if(m_context || m_contextFailed)
        return m_context;
    if(!mf::isPackagedProcess()) {
        m_contextFailed = true;
        TRC_INFO("store: not packaged, Store APIs off");
        return nullptr;
    }
    try {
        m_context = StoreContext::GetDefault();
        subscribeLicenseChanges();
        TRC_INFO("store: context ready");
    } catch(const winrt::hresult_error& e) {
        m_contextFailed = true;
        TRC_WARN("store: GetDefault failed ({}): {}", hresultText(e).toStdString(),
                 toQString(e.message()).toStdString());
    }
    return m_context;
}

void StoreLicensing::subscribeLicenseChanges() {
    m_licenseRevoker = m_context.OfflineLicensesChanged(
        winrt::auto_revoke, [this](const StoreContext&, const winrt::Windows::Foundation::IInspectable&) {
            TRC_INFO("store: offline licenses changed");
            QMetaObject::invokeMethod(this, [this] { emit licensesChanged(); }, Qt::QueuedConnection);
        });
}

StoreSnapshot StoreLicensing::readSnapshot(StoreContext ctx) const {
    StoreSnapshot s;
    try {
        const StoreAppLicense license = ctx.GetAppLicenseAsync().get();
        if(!license) {
            s.error = QStringLiteral("noAppLicense");
            TRC_WARN("store: GetAppLicenseAsync returned null");
            return s;
        }
        s.appLicenseActive = license.IsActive();
        s.appLicenseTrial = license.IsTrial();
        TRC_INFO("store: app license active={} trial={} sku={} expires={}", s.appLicenseActive, s.appLicenseTrial,
                 toQString(license.SkuStoreId()).toStdString(),
                 toQDateTime(license.ExpirationDate()).toString(Qt::ISODate).toStdString());
        for(const auto& kv : license.AddOnLicenses()) {
            const QString key = toQString(kv.Key());
            const StoreLicense addOn = kv.Value();
            const bool active = addOn.IsActive();
            TRC_DEBUG("store: add-on license key={} sku={} active={}", key.toStdString(),
                      toQString(addOn.SkuStoreId()).toStdString(), active);
            if(active && (licenseKeyMatches(key, m_proAddOnStoreId)
                          || licenseKeyMatches(toQString(addOn.SkuStoreId()), m_proAddOnStoreId)))
                s.addOnLicensed = true;
        }
        s.available = true;
    } catch(const winrt::hresult_error& e) {
        s.error = hresultText(e);
        TRC_WARN("store: GetAppLicenseAsync failed ({}): {}", s.error.toStdString(),
                 toQString(e.message()).toStdString());
        return s;
    }

    // Needs network and a signed-in account. A miss keeps the license result.
    // GetUserCollectionAsync does not list the app itself for a Desktop Bridge
    // package (verified: zero products with an active license), so the acquire
    // date comes from the current-app product's owned SKUs.
    try {
        const StoreProductResult result = ctx.GetStoreProductForCurrentAppAsync().get();
        const StoreProduct product = result.Product();
        if(!product) {
            TRC_WARN("store: GetStoreProductForCurrentAppAsync returned no product (extendedError=0x{:08x})",
                     static_cast<unsigned long>(result.ExtendedError().value));
        } else {
            for(const StoreSku& sku : product.Skus()) {
                const bool owned = sku.IsInUserCollection();
                const QDateTime acquired = owned ? toQDateTime(sku.CollectionData().AcquiredDate()) : QDateTime();
                TRC_INFO("store: app sku={} owned={} trial={} acquired={}", toQString(sku.StoreId()).toStdString(),
                         owned, sku.IsTrial(),
                         acquired.isValid() ? acquired.toString(Qt::ISODate).toStdString() : "none");
                if(owned && acquired.isValid() && (!s.acquired.isValid() || acquired < s.acquired))
                    s.acquired = acquired;
            }
        }
    } catch(const winrt::hresult_error& e) {
        TRC_WARN("store: GetStoreProductForCurrentAppAsync failed ({}): {}", hresultText(e).toStdString(),
                 toQString(e.message()).toStdString());
    }

    if(m_proAddOnStoreId.isEmpty() && m_consumableStoreIds.isEmpty())
        return s;
    try {
        const StoreProductQueryResult result =
            ctx.GetAssociatedStoreProductsAsync(kinds({L"Durable", L"Consumable"})).get();
        for(const auto& kv : result.Products()) {
            const QString key = toQString(kv.Key());
            const StoreProduct product = kv.Value();
            const QString price = toQString(product.Price().FormattedPrice());
            TRC_DEBUG("store: add-on product {} price={} owned={}", key.toStdString(), price.toStdString(),
                      product.IsInUserCollection());
            s.addOnPrices.insert(key, price);
            if(licenseKeyMatches(key, m_proAddOnStoreId)) {
                s.proPrice = price;
                if(product.IsInUserCollection())
                    s.addOnLicensed = true;
            }
        }
    } catch(const winrt::hresult_error& e) {
        TRC_WARN("store: GetAssociatedStoreProductsAsync failed ({}): {}", hresultText(e).toStdString(),
                 toQString(e.message()).toStdString());
    }
    return s;
}

void StoreLicensing::refresh() {
    StoreContext ctx = context();
    if(!ctx) {
        StoreSnapshot s;
        s.error = mf::isPackagedProcess() ? QStringLiteral("noContext") : QStringLiteral("notPackaged");
        emit snapshotReady(s);
        return;
    }
    if(m_refreshing) {
        m_refreshPending = true;
        return;
    }
    m_refreshing = true;

    // Blocking WinRT must not run on the STA UI thread.
    QThread* thread = QThread::create([this, ctx] {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
        const StoreSnapshot snapshot = readSnapshot(ctx);
        winrt::uninit_apartment();
        QMetaObject::invokeMethod(
            this,
            [this, snapshot] {
                m_refreshing = false;
                TRC_INFO("store: snapshot available={} addOn={} acquired={} price='{}' error='{}'",
                         snapshot.available, snapshot.addOnLicensed,
                         snapshot.acquired.isValid() ? snapshot.acquired.toString(Qt::ISODate).toStdString() : "none",
                         snapshot.proPrice.toStdString(), snapshot.error.toStdString());
                emit snapshotReady(snapshot);
                if(m_refreshPending) {
                    m_refreshPending = false;
                    refresh();
                }
            },
            Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_worker = thread;
    thread->start();
}

void StoreLicensing::purchase(const QString& storeId) {
    const QString id = storeId.trimmed();
    if(id.isEmpty()) {
        emit purchaseFinished(storeId, false, mf::purchase::kNotConfigured);
        return;
    }
    StoreContext ctx = context();
    if(!ctx) {
        emit purchaseFinished(id, false, mf::purchase::kUnavailable);
        return;
    }
    if(m_purchasing) {
        emit purchaseFinished(id, false, mf::purchase::kBusy);
        return;
    }
    m_purchasing = true;
    try {
        // Desktop Bridge: the Store dialog needs an owner HWND.
        if(m_hwnd)
            ctx.as<IInitializeWithWindow>()->Initialize(static_cast<HWND>(m_hwnd));
        else
            TRC_WARN("store: purchase without an owner window");

        TRC_INFO("store: requesting purchase of {}", id.toStdString());
        IAsyncOperation<StorePurchaseResult> op = ctx.RequestPurchaseAsync(winrt::hstring(id.toStdWString()));
        op.Completed([this, id](const IAsyncOperation<StorePurchaseResult>& sender, AsyncStatus status) {
            bool ok = false;
            QString statusId = mf::purchase::kServerError;
            try {
                if(status == AsyncStatus::Completed) {
                    const StorePurchaseResult result = sender.GetResults();
                    statusId = statusIdFor(result.Status(), &ok);
                    TRC_INFO("store: purchase {} -> {} (extended 0x{:08x})", id.toStdString(), statusId.toStdString(),
                             static_cast<unsigned long>(result.ExtendedError().value));
                } else {
                    TRC_WARN("store: purchase {} async status {}", id.toStdString(), static_cast<int>(status));
                }
            } catch(const winrt::hresult_error& e) {
                TRC_WARN("store: purchase {} failed ({}): {}", id.toStdString(), hresultText(e).toStdString(),
                         toQString(e.message()).toStdString());
            }
            QMetaObject::invokeMethod(
                this,
                [this, id, ok, statusId] {
                    m_purchasing = false;
                    if(ok && statusId == mf::purchase::kSucceeded && m_consumableStoreIds.contains(id))
                        fulfillConsumable(m_context, id);
                    emit purchaseFinished(id, ok, statusId);
                },
                Qt::QueuedConnection);
        });
    } catch(const winrt::hresult_error& e) {
        m_purchasing = false;
        TRC_WARN("store: RequestPurchaseAsync threw ({}): {}", hresultText(e).toStdString(),
                 toQString(e.message()).toStdString());
        emit purchaseFinished(id, false, mf::purchase::kServerError);
    }
}

void StoreLicensing::fulfillConsumable(StoreContext ctx, const QString& storeId) {
    if(!ctx)
        return;
    try {
        const winrt::guid tracking = winrt::Windows::Foundation::GuidHelper::CreateNewGuid();
        auto op = ctx.ReportConsumableFulfillmentAsync(winrt::hstring(storeId.toStdWString()), 1, tracking);
        op.Completed([storeId](const IAsyncOperation<StoreConsumableResult>& sender, AsyncStatus status) {
            try {
                if(status == AsyncStatus::Completed) {
                    const StoreConsumableResult result = sender.GetResults();
                    TRC_INFO("store: consumable {} fulfilled status={} balance={}", storeId.toStdString(),
                             static_cast<int>(result.Status()), result.BalanceRemaining());
                } else {
                    TRC_WARN("store: consumable {} fulfilment async status {}", storeId.toStdString(),
                             static_cast<int>(status));
                }
            } catch(const winrt::hresult_error& e) {
                TRC_WARN("store: consumable {} fulfilment failed ({}): {}", storeId.toStdString(),
                         hresultText(e).toStdString(), toQString(e.message()).toStdString());
            }
        });
    } catch(const winrt::hresult_error& e) {
        TRC_WARN("store: ReportConsumableFulfillmentAsync threw ({}): {}", hresultText(e).toStdString(),
                 toQString(e.message()).toStdString());
    }
}
