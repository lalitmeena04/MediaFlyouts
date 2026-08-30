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
#include "BatteryProviders/LogitechBatteryProvider.h"
#include "BatteryProviders/PlayStationControllerBatteryProvider.h"
#include "BatteryProviders/RazerBatteryProvider.h"
#include "BatteryProviders/WirelessReceiverProvider.h"

BatteryStatusController::BatteryStatusController(QObject* parent) : QObject(parent) {
    // Fast Pair first: it reports live per-component levels, so it wins the name-based
    // dedupe in applyPolled() over the coarser Hands-Free value for the same device
    m_providers.push_back(std::make_unique<FastPairBatteryProvider>());
    m_providers.push_back(std::make_unique<BluetoothClassicBatteryProvider>());
    m_providers.push_back(std::make_unique<BluetoothLeBatteryProvider>());
    m_providers.push_back(std::make_unique<GameControllerBatteryProvider>());
    m_providers.push_back(std::make_unique<PlayStationControllerBatteryProvider>());
    // 2.4GHz dongle devices, which have no standard battery channel
    m_providers.push_back(std::make_unique<RazerBatteryProvider>());
    m_providers.push_back(std::make_unique<LogitechBatteryProvider>());
    m_providers.push_back(std::make_unique<WirelessReceiverProvider>());
    m_pollOnWorkerThread = true;

    // poll as a fallback (battery % drifts without a device event, and this catches
    // anything the watchers below miss); connect/disconnect itself reacts instantly
    m_timer.setInterval(15 * 1000);
    connect(&m_timer, &QTimer::timeout, this, &BatteryStatusController::refresh);
    m_timer.start();

    // react immediately when a Bluetooth or HID device connects/disconnects, instead of
    // waiting for the next timer tick
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
    // real providers call blocking WinRT APIs (GATT reads, ...); the Qt main thread is a
    // single-threaded apartment (see main.cpp), and cppwinrt asserts/deadlocks if you block-wait
    // on an STA thread, so real polling happens on a plain worker thread instead.
    if(m_pollOnWorkerThread) {
        if(m_refreshing) {
            // don't drop the event: a device just connected or disconnected, so the
            // in-flight snapshot is already stale
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

    // test-injection path: fake providers do no real I/O, keep this synchronous for deterministic tests
    QVector<BatteryDeviceInfo> polled;
    for(auto& provider : m_providers)
        polled += provider->poll();
    applyPolled(polled);
}

void BatteryStatusController::applyPolled(const QVector<BatteryDeviceInfo>& polled) {
    QVariantList devices;
    QSet<QString> seen;
    for(const BatteryDeviceInfo& info : polled) {
        // the same headset can be reported by more than one provider (e.g. Fast Pair and
        // Hands-Free); keep the first, which is the richest source
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


