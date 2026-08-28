#include "BatteryStatusController.h"

#include <QVariantMap>

#include "BatteryProviders/BluetoothClassicBatteryProvider.h"
#include "BatteryProviders/BluetoothLeBatteryProvider.h"
#include "BatteryProviders/GameControllerBatteryProvider.h"
#include "BatteryProviders/IBatteryProvider.h"

BatteryStatusController::BatteryStatusController(QObject* parent) : QObject(parent) {
    m_providers.push_back(std::make_unique<BluetoothClassicBatteryProvider>());
    m_providers.push_back(std::make_unique<BluetoothLeBatteryProvider>());
    m_providers.push_back(std::make_unique<GameControllerBatteryProvider>());

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
    QVariantList devices;
    for(auto& provider : m_providers) {
        for(const BatteryDeviceInfo& info : provider->poll()) {
            QVariantMap entry;
            entry[QStringLiteral("name")] = info.name;
            entry[QStringLiteral("percent")] = info.percent;
            entry[QStringLiteral("connectionKind")] = info.connectionKind;
            devices.append(entry);
        }
    }

    if(devices != m_devices) {
        m_devices = devices;
        emit devicesChanged();
    }
}
