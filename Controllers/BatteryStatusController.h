#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>

#include <memory>
#include <vector>

#include "BluetoothConnectionWatcher.h"
#include "HidChangeWatcher.h"

class IBatteryProvider;

// Aggregates battery level for wireless peripherals (Bluetooth headsets/mice/keyboards,
// with more connection kinds to follow) into a single list exposed to QML. Devices a
// provider can't read a battery level for are simply omitted, never shown as 0%/unknown.
class BatteryStatusController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

    public:
        explicit BatteryStatusController(QObject* parent = nullptr);
        // test seam: inject providers directly instead of the real Windows-backed ones
        explicit BatteryStatusController(std::vector<std::unique_ptr<IBatteryProvider>> providers,
                                          QObject* parent = nullptr);
        ~BatteryStatusController() override;

        QVariantList devices() const;

    public slots:
        // re-polls all providers now; also called automatically on a timer
        void refresh();

    signals:
        void devicesChanged();

    private:
        // applies a freshly polled snapshot on the main thread (updates m_devices, emits if changed)
        void applyPolled(const QVector<struct BatteryDeviceInfo>& polled);

        std::vector<std::unique_ptr<IBatteryProvider>> m_providers;
        QVariantList m_devices;
        QTimer m_timer;
        // instant connect/disconnect signals layered on top of the timer, which still
        // catches battery-level drift and anything the watchers miss
        BluetoothConnectionWatcher m_bluetoothWatcher;
        HidChangeWatcher m_hidWatcher;
        // real (Windows) providers poll on a background thread to avoid blocking the STA main
        // thread on WinRT async calls; the test-injection constructor leaves this false
        bool m_pollOnWorkerThread = false;
        bool m_refreshing = false;
        // a connect/disconnect seen while a poll is in flight, re-run once it finishes
        bool m_refreshPending = false;
};
