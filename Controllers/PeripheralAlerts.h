#pragma once

#include <QObject>
#include <QVariantList>

#include "BatteryAlertPolicy.h"

class BatteryStatusController;

// Convert peripheral snapshots into user alerts.
// BatteryAlertPolicy owns decision rules. This class connects Qt and QML.
class PeripheralAlerts : public QObject {
        Q_OBJECT
    // Alerts from most recent snapshot. Flyout displays this list.
        Q_PROPERTY(QVariantList pending READ pending NOTIFY alertsRaised)

    public:
        explicit PeripheralAlerts(BatteryStatusController* status, QObject* parent = nullptr);

        QVariantList pending() const;

        Q_INVOKABLE void clear();

        // Battery bucket label for a percentage.
        Q_INVOKABLE static QString bucketLabel(int percent);

    signals:
        // Emitted once per snapshot with at least one alert.
        void alertsRaised();
        // Low battery also goes to tray notification.
        void lowBattery(const QString& deviceName, int percent, bool veryLow);

    private slots:
        void onDevicesChanged();

    private:
        BatteryStatusController* m_status = nullptr;
        mf::BatteryAlertPolicy m_policy;
        QVariantList m_pending;
};
