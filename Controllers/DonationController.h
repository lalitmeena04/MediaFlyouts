#pragma once

#include <QObject>
#include <QString>

#include "Donation.h"
#include "InstallSource.h"

// The `donate` QML context property. Which surfaces show depends on the install kind.
class DonationController : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool available READ available CONSTANT)
        Q_PROPERTY(bool showDirect READ showDirect CONSTANT)
        Q_PROPERTY(bool showStoreTips READ showStoreTips CONSTANT)
        Q_PROPERTY(bool hasPage READ hasPage CONSTANT)
        Q_PROPERTY(bool hasUpi READ hasUpi CONSTANT)
        Q_PROPERTY(bool hasBank READ hasBank CONSTANT)
        Q_PROPERTY(QString pageUrl READ pageUrl CONSTANT)
        Q_PROPERTY(QString upiId READ upiId CONSTANT)
        Q_PROPERTY(QString payeeName READ payeeName CONSTANT)
        Q_PROPERTY(QString upiUri READ upiUri CONSTANT)
        Q_PROPERTY(QString upiQrSource READ upiQrSource CONSTANT)
        Q_PROPERTY(QString bankName READ bankName CONSTANT)
        Q_PROPERTY(QString bankAccount READ bankAccount CONSTANT)
        Q_PROPERTY(QString bankIfsc READ bankIfsc CONSTANT)
        Q_PROPERTY(QString bankHolder READ bankHolder CONSTANT)

    public:
        DonationController(const mf::DonationConfig& config, mf::InstallKind install, bool storeTipsConfigured,
                           QObject* parent = nullptr);

        bool available() const;
        bool showDirect() const;
        bool showStoreTips() const;
        bool hasPage() const;
        bool hasUpi() const;
        bool hasBank() const;
        QString pageUrl() const;
        QString upiId() const;
        QString payeeName() const;
        QString upiUri() const;
        QString upiQrSource() const;
        QString bankName() const;
        QString bankAccount() const;
        QString bankIfsc() const;
        QString bankHolder() const;

        Q_INVOKABLE bool openPage() const;
        Q_INVOKABLE void copyToClipboard(const QString& text) const;

        static QString qrSourceFor(const QString& text);

    private:
        mf::DonationConfig m_config;
        mf::DonationLayout m_layout;
        bool m_storeTipsConfigured;
};
