#include "Donation.h"

#include <QUrl>

#include "Logging.h"

namespace mf {

bool DonationConfig::hasPage() const {
    return !pageUrl.isEmpty();
}

bool DonationConfig::hasUpi() const {
    return !upiId.isEmpty();
}

bool DonationConfig::hasBank() const {
    return !bankAccount.isEmpty() && !bankIfsc.isEmpty();
}

bool DonationConfig::any() const {
    return hasPage() || hasUpi() || hasBank();
}

DonationConfig parseDonationConfig(const QString& pageUrl, const QString& upiId, const QString& payeeName,
                                   const QString& bankName, const QString& bankAccount, const QString& bankIfsc,
                                   const QString& bankHolder) {
    DonationConfig c;
    const QUrl url(pageUrl.trimmed());
    if(url.isValid() && (url.scheme() == QLatin1String("https") || url.scheme() == QLatin1String("http")))
        c.pageUrl = url.toString();
    else if(!pageUrl.trimmed().isEmpty())
        TRC_WARN("donation: page url '{}' is not http(s), ignored", pageUrl.toStdString());
    c.upiId = upiId.trimmed();
    if(!c.upiId.isEmpty() && !c.upiId.contains(QLatin1Char('@'))) {
        TRC_WARN("donation: upi id '{}' has no '@', ignored", c.upiId.toStdString());
        c.upiId.clear();
    }
    c.payeeName = payeeName.trimmed();
    c.bankName = bankName.trimmed();
    c.bankAccount = bankAccount.trimmed();
    c.bankIfsc = bankIfsc.trimmed().toUpper();
    c.bankHolder = bankHolder.trimmed();
    return c;
}

DonationConfig donationConfig() {
    static const DonationConfig config = [] {
        const DonationConfig c = parseDonationConfig(
            QString::fromUtf8(MEDIAFLYOUTS_DONATE_URL), QString::fromUtf8(MEDIAFLYOUTS_DONATE_UPI_ID),
            QString::fromUtf8(MEDIAFLYOUTS_DONATE_PAYEE_NAME), QString::fromUtf8(MEDIAFLYOUTS_DONATE_BANK_NAME),
            QString::fromUtf8(MEDIAFLYOUTS_DONATE_BANK_ACCOUNT), QString::fromUtf8(MEDIAFLYOUTS_DONATE_BANK_IFSC),
            QString::fromUtf8(MEDIAFLYOUTS_DONATE_BANK_HOLDER));
        TRC_INFO("donation: page={} upi={} bank={}", c.hasPage(), c.hasUpi(), c.hasBank());
        return c;
    }();
    return config;
}

DonationLayout donationLayoutFor(InstallKind kind) {
    return kind == InstallKind::Store ? DonationLayout::StoreLink : DonationLayout::Direct;
}

QString upiPaymentUri(const QString& vpa, const QString& payee, const QString& note) {
    const QString id = vpa.trimmed();
    if(id.isEmpty())
        return {};
    const auto enc = [](const QString& s, const QByteArray& keep = QByteArray()) {
        return QString::fromLatin1(QUrl::toPercentEncoding(s, keep));
    };
    QString uri = QStringLiteral("upi://pay?pa=") + enc(id, "@");
    if(!payee.trimmed().isEmpty())
        uri += QStringLiteral("&pn=") + enc(payee.trimmed());
    uri += QStringLiteral("&cu=INR");
    if(!note.trimmed().isEmpty())
        uri += QStringLiteral("&tn=") + enc(note.trimmed());
    return uri;
}

} // namespace mf
