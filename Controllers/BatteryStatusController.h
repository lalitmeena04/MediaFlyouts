#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>

#include <memory>
#include <vector>

#include "BluetoothConnectionWatcher.h"
#include "HidChangeWatcher.h"

class IBatteryProvider;

// Aggregate battery levels for wireless peripherals. Expose list to QML.
// Include devices with unknown battery. Alerts still work.
class BatteryStatusController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

    public:
        explicit BatteryStatusController(QObject* parent = nullptr);
        // Test path. Inject providers without Windows hardware APIs.
        explicit BatteryStatusController(std::vector<std::unique_ptr<IBatteryProvider>> providers,
                                          QObject* parent = nullptr);
        ~BatteryStatusController() override;

        QVariantList devices() const;

    public slots:
        // Re-poll all providers now. Timer also calls this.
        void refresh();

    signals:
        void devicesChanged();

    private:
        // Apply polled snapshot on main thread. Emit if changed.
        void applyPolled(const QVector<struct BatteryDeviceInfo>& polled);

        std::vector<std::unique_ptr<IBatteryProvider>> m_providers;
        QVariantList m_devices;
        QTimer m_timer;
        // Instant connect/disconnect signals. Timer handles drift and missed events.
        BluetoothConnectionWatcher m_bluetoothWatcher;
        HidChangeWatcher m_hidWatcher;
        // Windows providers poll on background thread. Test path leaves this false.
        bool m_pollOnWorkerThread = false;
        bool m_refreshing = false;
        // Device changed during poll. Re-run after poll finishes.
        bool m_refreshPending = false;
};
