#include "BatteryStatusController.h"

#include <QThread>
#include <QSet>
#include <QVariantMap>

#include <winrt/base.h>

#include "BatteryProviders/BluetoothClassicBatteryProvider.h"
#include "BatteryProviders/BluetoothLeBatteryProvider.h"
#include "BatteryProviders/FastPairBatteryProvider.h"
#include "BatteryProviders/GameControllerBatteryProvider.h"
#include "BatteryProviders/IBatteryProvider.h"
#include "BatteryProviders/JabraBatteryProvider.h"
#include "BatteryProviders/LogitechBatteryProvider.h"
#include "BatteryProviders/PlayStationControllerBatteryProvider.h"
#include "BatteryProviders/RazerBatteryProvider.h"
#include "BatteryProviders/WiredPeripheralProvider.h"
#include "BatteryProviders/WirelessReceiverProvider.h"

BatteryStatusController::BatteryStatusController(QObject* parent) : QObject(parent) {
    // Add Fast Pair first. It has higher priority. It reports live component levels.
    m_providers.push_back(std::make_unique<FastPairBatteryProvider>());
    m_providers.push_back(std::make_unique<BluetoothClassicBatteryProvider>());
    m_providers.push_back(std::make_unique<BluetoothLeBatteryProvider>());
    m_providers.push_back(std::make_unique<GameControllerBatteryProvider>());
    m_providers.push_back(std::make_unique<PlayStationControllerBatteryProvider>());
    m_providers.push_back(std::make_unique<RazerBatteryProvider>());
    m_providers.push_back(std::make_unique<LogitechBatteryProvider>());
    m_providers.push_back(std::make_unique<JabraBatteryProvider>());
    m_providers.push_back(std::make_unique<WirelessReceiverProvider>());
    // Wired devices carry no battery. Listed last so a wireless reading always wins the name.
    m_providers.push_back(std::make_unique<WiredPeripheralProvider>());
    m_pollOnWorkerThread = true;

    m_timer.setInterval(15 * 1000);
    connect(&m_timer, &QTimer::timeout, this, &BatteryStatusController::refresh);
    m_timer.start();

    connect(&m_bluetoothWatcher, &BluetoothConnectionWatcher::deviceChanged, this,
            &BatteryStatusController::refresh);
    connect(&m_hidWatcher, &HidChangeWatcher::deviceChanged, this, &BatteryStatusController::refresh);
    m_bluetoothWatcher.start();
    m_hidWatcher.start();

    refresh();
}

BatteryStatusController::BatteryStatusController(std::vector<std::unique_ptr<IBatteryProvider>> providers,
                                                   QObject* parent)
    : QObject(parent), m_providers(std::move(providers)) {
    refresh();
}

BatteryStatusController::~BatteryStatusController() = default;

QVariantList BatteryStatusController::devices() const {
    return m_devices;
}

void BatteryStatusController::refresh() {
    // Real providers use blocking WinRT APIs. Qt main thread is STA.
    if(m_pollOnWorkerThread) {
        if(m_refreshing) {
            m_refreshPending = true;
            return;
        }
        m_refreshing = true;

        auto* providers = &m_providers;
        QThread* thread = QThread::create([this, providers] {
            winrt::init_apartment(winrt::apartment_type::multi_threaded);
            QVector<BatteryDeviceInfo> polled;
            for(auto& provider : *providers)
                polled += provider->poll();
            winrt::uninit_apartment();

            QMetaObject::invokeMethod(
                this,
                [this, polled] {
                    m_refreshing = false;
                    applyPolled(polled);
                    if(m_refreshPending) {
                        m_refreshPending = false;
                        refresh();
                    }
                },
                Qt::QueuedConnection);
        });
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();
        return;
    }

    QVector<BatteryDeviceInfo> polled;
    for(auto& provider : m_providers)
        polled += provider->poll();
    applyPolled(polled);
}

void BatteryStatusController::applyPolled(const QVector<BatteryDeviceInfo>& polled) {
    QVariantList devices;
    QSet<QString> seen;
    for(const BatteryDeviceInfo& info : polled) {
        if(seen.contains(info.name))
            continue;
        seen.insert(info.name);

        QVariantMap entry;
        entry[QStringLiteral("name")] = info.name;
        entry[QStringLiteral("percent")] = info.percent;
        entry[QStringLiteral("connectionKind")] = info.connectionKind;
        entry[QStringLiteral("category")] = info.category;
        entry[QStringLiteral("charging")] = info.charging;
        QVariantList components;
        for(const BatteryComponentInfo& component : info.components) {
            QVariantMap part;
            part[QStringLiteral("label")] = component.label;
            part[QStringLiteral("percent")] = component.percent;
            part[QStringLiteral("charging")] = component.charging;
            components.append(part);
        }
        entry[QStringLiteral("components")] = components;
        devices.append(entry);
    }

    if(devices != m_devices) {
        m_devices = devices;
        emit devicesChanged();
    }
}


