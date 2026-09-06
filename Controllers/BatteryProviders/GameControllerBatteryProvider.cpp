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
            int percent = -1;
            BatteryReport report = controller.TryGetBatteryReport();
            if(report) {
                const auto remaining = report.RemainingCapacityInMilliwattHours();
                const auto full = report.FullChargeCapacityInMilliwattHours();
                if(remaining && full)
                    percent = BatteryUtils::milliwattHoursToPercent(remaining.Value(), full.Value());
            }

            QString name = QString::fromWCharArray(controller.DisplayName().c_str());
            if(name.isEmpty())
                name = QStringLiteral("Game controller");

            // Report wired or non-reporting controllers with unknown battery.
            // Connect/disconnect events still surface.
            result.append({name, percent, QStringLiteral("gamepad")});
        }
    } catch(...) {
        // Battery reporting is best effort. Skip this cycle on WinRT failure.
    }

    return result;
}
