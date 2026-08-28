#include "BatteryStatusController.h"

#include <QThread>
#include <QVariantMap>

#include <winrt/base.h>

#include "BatteryProviders/BluetoothClassicBatteryProvider.h"
#include "BatteryProviders/BluetoothLeBatteryProvider.h"
#include "BatteryProviders/GameControllerBatteryProvider.h"
#include "BatteryProviders/IBatteryProvider.h"
#include "BatteryProviders/PlayStationControllerBatteryProvider.h"

BatteryStatusController::BatteryStatusController(QObject* parent) : QObject(parent) {
    m_providers.push_back(std::make_unique<BluetoothClassicBatteryProvider>());
    m_providers.push_back(std::make_unique<BluetoothLeBatteryProvider>());
    m_providers.push_back(std::make_unique<GameControllerBatteryProvider>());
    m_providers.push_back(std::make_unique<PlayStationControllerBatteryProvider>());
    m_pollOnWorkerThread = true;

    // battery level changes slowly; poll infrequently rather than watch for device events
    m_timer.setInterval(5 * 60 * 1000);
    connect(&m_timer, &QTimer::timeout, this, &BatteryStatusController::refresh);
    m_timer.start();

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
        if(m_refreshing)
            return;
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
    for(const BatteryDeviceInfo& info : polled) {
        QVariantMap entry;
        entry[QStringLiteral("name")] = info.name;
        entry[QStringLiteral("percent")] = info.percent;
        entry[QStringLiteral("connectionKind")] = info.connectionKind;
        devices.append(entry);
    }

    if(devices != m_devices) {
        m_devices = devices;
        emit devicesChanged();
    }
}

