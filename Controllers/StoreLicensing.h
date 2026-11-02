#pragma once

#include <QPointer>
#include <QString>
#include <QThread>

#include <winrt/Windows.Services.Store.h>

#include "StoreGateway.h"

// Reads run on a worker thread (blocking WinRT). The purchase dialog runs on the UI thread.
class StoreLicensing : public StoreGateway {
        Q_OBJECT
    public:
        explicit StoreLicensing(const QString& proAddOnStoreId, QObject* parent = nullptr);
        ~StoreLicensing() override;

        bool supported() const override;
        void setWindow(QWindow* window) override;

        QString proAddOnStoreId() const;

        static QString statusIdFor(winrt::Windows::Services::Store::StorePurchaseStatus status, bool* ok);
        // Keys may carry a SKU suffix: "9NXXXX/0010".
        static bool licenseKeyMatches(const QString& key, const QString& storeId);

    public slots:
        void refresh() override;
        void purchase(const QString& storeId) override;

    private:
        winrt::Windows::Services::Store::StoreContext context();
        void subscribeLicenseChanges();
        StoreSnapshot readSnapshot(winrt::Windows::Services::Store::StoreContext ctx) const;

        QString m_proAddOnStoreId;
        winrt::Windows::Services::Store::StoreContext m_context{nullptr};
        winrt::Windows::Services::Store::StoreContext::OfflineLicensesChanged_revoker m_licenseRevoker;
        bool m_contextFailed = false;
        bool m_refreshing = false;
        bool m_refreshPending = false;
        bool m_purchasing = false;
        QPointer<QThread> m_worker;
        void* m_hwnd = nullptr;
};
