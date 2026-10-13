#include "PeripheralAlerts.h"

#include "BatteryStatusController.h"

#include <QDateTime>
#include <QVariantMap>

namespace {

QString eventName(mf::PeripheralEvent event) {
    switch(event) {
    case mf::PeripheralEvent::Connected:       return QStringLiteral("connected");
    case mf::PeripheralEvent::Disconnected:    return QStringLiteral("disconnected");
    case mf::PeripheralEvent::LowBattery:      return QStringLiteral("lowBattery");
    case mf::PeripheralEvent::VeryLowBattery:  return QStringLiteral("veryLowBattery");
    case mf::PeripheralEvent::ChargingStarted: return QStringLiteral("chargingStarted");
    case mf::PeripheralEvent::ChargingStopped: return QStringLiteral("chargingStopped");
    }
    return {};
}

} // namespace

PeripheralAlerts::PeripheralAlerts(BatteryStatusController* status, QObject* parent)
    : QObject(parent), m_status(status) {
    if(m_status) {
        connect(m_status, &BatteryStatusController::devicesChanged, this, &PeripheralAlerts::onDevicesChanged);
        onDevicesChanged();
    }
}

QVariantList PeripheralAlerts::pending() const {
    return m_pending;
}

void PeripheralAlerts::clear() {
    if(m_pending.isEmpty())
        return;
    m_pending.clear();
    emit alertsRaised();
}

QString PeripheralAlerts::bucketLabel(int percent) {
    return mf::bucketLabel(mf::bucketOf(percent));
}

void PeripheralAlerts::onDevicesChanged() {
    if(!m_status)
        return;

    QVector<mf::PeripheralSnapshot> snapshot;
    const QVariantList devices = m_status->devices();
    for(const QVariant& entry : devices) {
        const QVariantMap device = entry.toMap();
        mf::PeripheralSnapshot item;
        item.name = device.value(QStringLiteral("name")).toString();
        item.percent = device.value(QStringLiteral("percent"), -1).toInt();
        item.charging = device.value(QStringLiteral("charging")).toBool();
        item.category = device.value(QStringLiteral("category")).toString();
        item.connectionKind = device.value(QStringLiteral("connectionKind")).toString();
        snapshot.append(item);
    }

    const QVector<mf::PeripheralAlert> alerts =
        m_policy.update(snapshot, QDateTime::currentMSecsSinceEpoch());
    if(alerts.isEmpty())
        return;

    m_pending.clear();
    for(const mf::PeripheralAlert& alert : alerts) {
        QVariantMap entry;
        entry[QStringLiteral("event")] = eventName(alert.event);
        entry[QStringLiteral("name")] = alert.deviceName;
        entry[QStringLiteral("percent")] = alert.percent;
        entry[QStringLiteral("bucket")] = mf::bucketLabel(alert.bucket);
        entry[QStringLiteral("category")] = alert.category;
        entry[QStringLiteral("connectionKind")] = alert.connectionKind;
        m_pending.append(entry);

        if(alert.event == mf::PeripheralEvent::LowBattery || alert.event == mf::PeripheralEvent::VeryLowBattery)
            emit lowBattery(alert.deviceName, alert.percent,
                            alert.event == mf::PeripheralEvent::VeryLowBattery);
    }
    emit alertsRaised();
}
