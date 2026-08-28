#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>

#include <memory>
#include <vector>

class IBatteryProvider;

// Aggregates battery level for wireless peripherals (Bluetooth headsets/mice/keyboards,
// with more connection kinds to follow) into a single list exposed to QML. Devices a
// provider can't read a battery level for are simply omitted, never shown as 0%/unknown.
class BatteryStatusController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

    public:
        explicit BatteryStatusController(QObject* parent = nullptr);
        ~BatteryStatusController() override;

        QVariantList devices() const;

    public slots:
        // re-polls all providers now; also called automatically on a timer
        void refresh();

    signals:
        void devicesChanged();

    private:
        std::vector<std::unique_ptr<IBatteryProvider>> m_providers;
        QVariantList m_devices;
        QTimer m_timer;
};
