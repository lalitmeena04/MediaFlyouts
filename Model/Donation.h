#pragma once

#include <QString>

#include "InstallSource.h"

#ifndef MEDIAFLYOUTS_DONATE_URL
#define MEDIAFLYOUTS_DONATE_URL ""
#endif
#ifndef MEDIAFLYOUTS_DONATE_UPI_ID
#define MEDIAFLYOUTS_DONATE_UPI_ID ""
#endif
#ifndef MEDIAFLYOUTS_DONATE_PAYEE_NAME
#define MEDIAFLYOUTS_DONATE_PAYEE_NAME ""
#endif
#ifndef MEDIAFLYOUTS_DONATE_BANK_NAME
#define MEDIAFLYOUTS_DONATE_BANK_NAME ""
#endif
#ifndef MEDIAFLYOUTS_DONATE_BANK_ACCOUNT
#define MEDIAFLYOUTS_DONATE_BANK_ACCOUNT ""
#endif
#ifndef MEDIAFLYOUTS_DONATE_BANK_IFSC
#define MEDIAFLYOUTS_DONATE_BANK_IFSC ""
#endif
#ifndef MEDIAFLYOUTS_DONATE_BANK_HOLDER
#define MEDIAFLYOUTS_DONATE_BANK_HOLDER ""
#endif

namespace mf {

struct DonationConfig {
    QString pageUrl;
    QString upiId;
    QString payeeName;
    QString bankName;
    QString bankAccount;
    QString bankIfsc;
    QString bankHolder;

    bool hasPage() const;
    bool hasUpi() const;
    bool hasBank() const;
    bool any() const;
};

DonationConfig parseDonationConfig(const QString& pageUrl, const QString& upiId, const QString& payeeName,
                                   const QString& bankName, const QString& bankAccount, const QString& bankIfsc,
                                   const QString& bankHolder);
DonationConfig donationConfig();

// Which surfaces an install shows. Store policy allows only a link and Store add-ons in the Store build.
enum class DonationLayout {
    Direct,    // UPI QR, NEFT details and the page link
    StoreLink, // page link and consumable tip add-ons
};
DonationLayout donationLayoutFor(InstallKind kind);

// upi://pay?pa=<vpa>&pn=<payee>&cu=INR[&tn=<note>]. Empty when `vpa` is empty.
QString upiPaymentUri(const QString& vpa, const QString& payee, const QString& note = QString());

} // namespace mf
