#include "GameControllerBatteryProvider.h"

#include "BatteryUtils.h"

#include <winrt/Windows.Devices.Power.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Gaming.Input.h>

using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::Devices::Power;

QVector<BatteryDeviceInfo> GameControllerBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;

    try {
        for(const RawGameController& controller : RawGameController::RawGameControllers()) {
            BatteryReport report = controller.TryGetBatteryReport();
            if(!report)
                continue;

            const auto remaining = report.RemainingCapacityInMilliwattHours();
            const auto full = report.FullChargeCapacityInMilliwattHours();
            if(!remaining || !full)
                continue;

            const int percent = BatteryUtils::milliwattHoursToPercent(remaining.Value(), full.Value());
            if(percent < 0)
                continue;

            QString name = QString::fromWCharArray(controller.DisplayName().c_str());
            if(name.isEmpty())
                name = QStringLiteral("Game controller");

            result.append({name, percent, QStringLiteral("gamepad")});
        }
    } catch(...) {
        // battery reporting is best-effort; skip this cycle on any WinRT failure
    }

    return result;
}
