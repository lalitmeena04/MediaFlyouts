#pragma once

#include <QObject>
#include <QVariantList>

#include "BatteryAlertPolicy.h"

class BatteryStatusController;

class PeripheralAlerts : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList pending READ pending NOTIFY alertsRaised)

    public:
        explicit PeripheralAlerts(BatteryStatusController* status, QObject* parent = nullptr);

        QVariantList pending() const;

        Q_INVOKABLE void clear();

        Q_INVOKABLE static QString bucketLabel(int percent);

    signals:
        void alertsRaised();
        void lowBattery(const QString& deviceName, int percent, bool veryLow);

    private slots:
        void onDevicesChanged();

    private:
        BatteryStatusController* m_status = nullptr;
        mf::BatteryAlertPolicy m_policy;
        QVariantList m_pending;
};
