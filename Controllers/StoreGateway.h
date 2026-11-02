#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariantMap>

class QWindow;

// `available` false = the Store gave no answer; `error` says why.
struct StoreSnapshot {
    bool available = false;
    bool appLicenseActive = false;
    bool appLicenseTrial = false;
    bool addOnLicensed = false;
    QDateTime acquired;
    QString proPrice;
    QVariantMap addOnPrices; // store id -> formatted price
    QString error;

    bool operator==(const StoreSnapshot& o) const {
        return available == o.available && appLicenseActive == o.appLicenseActive
               && appLicenseTrial == o.appLicenseTrial && addOnLicensed == o.addOnLicensed && acquired == o.acquired
               && proPrice == o.proPrice && addOnPrices == o.addOnPrices && error == o.error;
    }
    bool operator!=(const StoreSnapshot& o) const { return !(*this == o); }
};

// Stable ids. QML maps them to text.
namespace mf::purchase {
inline const QString kSucceeded = QStringLiteral("succeeded");
inline const QString kAlreadyPurchased = QStringLiteral("alreadyPurchased");
inline const QString kNotPurchased = QStringLiteral("notPurchased");
inline const QString kNetworkError = QStringLiteral("networkError");
inline const QString kServerError = QStringLiteral("serverError");
inline const QString kUnavailable = QStringLiteral("unavailable");
inline const QString kNotConfigured = QStringLiteral("notConfigured");
inline const QString kBusy = QStringLiteral("busy");
} // namespace mf::purchase

// Async Store access. Every call answers through a signal.
class StoreGateway : public QObject {
        Q_OBJECT
    public:
        using QObject::QObject;

        virtual bool supported() const = 0;
        virtual void setWindow(QWindow* window) { Q_UNUSED(window); }

    public slots:
        virtual void refresh() = 0;
        virtual void purchase(const QString& storeId) = 0;

    signals:
        void snapshotReady(const StoreSnapshot& snapshot);
        void purchaseFinished(const QString& storeId, bool ok, const QString& statusId);
        void licensesChanged();
};