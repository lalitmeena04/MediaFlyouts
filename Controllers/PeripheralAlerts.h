#pragma once

#include <QObject>
#include <QVariantList>

#include "BatteryAlertPolicy.h"

class BatteryStatusController;

// Turns the peripheral snapshots published by BatteryStatusController into the events
// worth telling the user about: a device connected, disconnected, or its battery crossed
// a low threshold. The decision rules live in mf::BatteryAlertPolicy; this class only
// bridges them to Qt/QML.
class PeripheralAlerts : public QObject {
        Q_OBJECT
        // the alerts raised by the most recent snapshot, for the flyout to display
        Q_PROPERTY(QVariantList pending READ pending NOTIFY alertsRaised)

    public:
        explicit PeripheralAlerts(BatteryStatusController* status, QObject* parent = nullptr);

        QVariantList pending() const;

        Q_INVOKABLE void clear();

        // Human-readable battery bucket ("Full", "Low", ...) for a percentage.
        Q_INVOKABLE static QString bucketLabel(int percent);

    signals:
        // emitted once per snapshot that produced at least one alert
        void alertsRaised();
        // low/very-low battery separately: it can arrive at any moment, so it also goes
        // to a tray notification rather than only an on-screen flyout
        void lowBattery(const QString& deviceName, int percent, bool veryLow);

    private slots:
        void onDevicesChanged();

    private:
        BatteryStatusController* m_status = nullptr;
        mf::BatteryAlertPolicy m_policy;
        QVariantList m_pending;
};
