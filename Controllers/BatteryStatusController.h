#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>

#include <memory>
#include <vector>

#include "BluetoothConnectionWatcher.h"
#include "HidChangeWatcher.h"

class IBatteryProvider;

class BatteryStatusController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

    public:
        explicit BatteryStatusController(QObject* parent = nullptr);
        explicit BatteryStatusController(std::vector<std::unique_ptr<IBatteryProvider>> providers,
                                          QObject* parent = nullptr);
        ~BatteryStatusController() override;

        QVariantList devices() const;

    public slots:
        void refresh();

    signals:
        void devicesChanged();

    private:
        void applyPolled(const QVector<struct BatteryDeviceInfo>& polled);

        std::vector<std::unique_ptr<IBatteryProvider>> m_providers;
        QVariantList m_devices;
        QTimer m_timer;
        BluetoothConnectionWatcher m_bluetoothWatcher;
        HidChangeWatcher m_hidWatcher;
        bool m_pollOnWorkerThread = false;
        bool m_refreshing = false;
        bool m_refreshPending = false;
};
