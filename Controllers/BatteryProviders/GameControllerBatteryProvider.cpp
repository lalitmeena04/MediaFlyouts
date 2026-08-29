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

            // still reported (with an unknown battery) for wired/non-reporting controllers,
            // so their connect/disconnect events surface too
            result.append({name, percent, QStringLiteral("gamepad")});
        }
    } catch(...) {
        // battery reporting is best-effort; skip this cycle on any WinRT failure
    }

    return result;
}
