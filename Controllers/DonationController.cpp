#include "DonationController.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QUrl>

#include "Logging.h"

DonationController::DonationController(const mf::DonationConfig& config, mf::InstallKind install,
                                       bool storeTipsConfigured, QObject* parent)
    : QObject(parent), m_config(config), m_layout(mf::donationLayoutFor(install)),
      m_storeTipsConfigured(storeTipsConfigured) {
    TRC_INFO("donation: layout={} page={} upi={} bank={} tips={}",
             m_layout == mf::DonationLayout::Direct ? "direct" : "storeLink", hasPage(), hasUpi(), hasBank(),
             showStoreTips());
}

bool DonationController::available() const {
    return hasPage() || showDirect() || showStoreTips();
}

bool DonationController::showDirect() const {
    return m_layout == mf::DonationLayout::Direct && (hasUpi() || hasBank());
}

bool DonationController::showStoreTips() const {
    return m_layout == mf::DonationLayout::StoreLink && m_storeTipsConfigured;
}

bool DonationController::hasPage() const {
    return m_config.hasPage();
}

bool DonationController::hasUpi() const {
    return m_config.hasUpi();
}

bool DonationController::hasBank() const {
    return m_config.hasBank();
}

QString DonationController::pageUrl() const {
    return m_config.pageUrl;
}

QString DonationController::upiId() const {
    return m_config.upiId;
}

QString DonationController::payeeName() const {
    return m_config.payeeName;
}

QString DonationController::upiUri() const {
    return mf::upiPaymentUri(m_config.upiId, m_config.payeeName, QStringLiteral("MediaFlyouts donation"));
}

QString DonationController::upiQrSource() const {
    return hasUpi() ? qrSourceFor(upiUri()) : QString();
}

QString DonationController::bankName() const {
    return m_config.bankName;
}

QString DonationController::bankAccount() const {
    return m_config.bankAccount;
}

QString DonationController::bankIfsc() const {
    return m_config.bankIfsc;
}

QString DonationController::bankHolder() const {
    return m_config.bankHolder;
}

bool DonationController::openPage() const {
    if(!hasPage())
        return false;
    const bool ok = QDesktopServices::openUrl(QUrl(m_config.pageUrl));
    TRC_INFO("donation: open page {} -> {}", m_config.pageUrl.toStdString(), ok);
    return ok;
}

void DonationController::copyToClipboard(const QString& text) const {
    if(QClipboard* clipboard = QGuiApplication::clipboard())
        clipboard->setText(text);
}

QString DonationController::qrSourceFor(const QString& text) {
    // Base64url keeps the id free of characters that QUrl would re-decode.
    return QStringLiteral("image://qr/")
           + QString::fromLatin1(text.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}
