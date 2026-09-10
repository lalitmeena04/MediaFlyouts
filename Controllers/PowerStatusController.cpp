#include "PowerStatusController.h"

#include <windows.h>

#include "Logging.h"

namespace {

// GUID_ACDC_POWER_SOURCE.
// Hardcoded to avoid an extra import library for the power setting GUIDs.
constexpr GUID kAcDcPowerSourceGuid = {
    0x5d3e9a59, 0xe9d5, 0x4b00, {0xa6, 0xbd, 0xff, 0x34, 0xff, 0x51, 0x65, 0x48}};

constexpr wchar_t kWindowClassName[] = L"MediaFlyoutsPowerStatusController";

// SYSTEM_POWER_STATUS uses 255 for unknown and bit 128 for no battery.
constexpr BYTE kUnknownValue = 255;
constexpr BYTE kNoBatteryFlag = 128;

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg == WM_POWERBROADCAST && wParam == PBT_POWERSETTINGCHANGE) {
        auto* self = reinterpret_cast<PowerStatusController*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if(self)
            self->refresh();
        return TRUE;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

PowerStatusController::PowerStatusController(QObject* parent) : QObject(parent) {}

PowerStatusController::~PowerStatusController() {
    if(m_notificationHandle)
        UnregisterPowerSettingNotification(reinterpret_cast<HPOWERNOTIFY>(m_notificationHandle));
    if(m_hwnd)
        DestroyWindow(reinterpret_cast<HWND>(m_hwnd));
}

bool PowerStatusController::pluggedIn() const {
    return m_pluggedIn;
}

int PowerStatusController::percent() const {
    return m_percent;
}

QString PowerStatusController::bucket() const {
    return m_bucket;
}

bool PowerStatusController::available() const {
    return m_available;
}

void PowerStatusController::start() {
    static bool classRegistered = false;
    if(!classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = wndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kWindowClassName;
        RegisterClassExW(&wc);
        classRegistered = true;
    }

    HWND hwnd = CreateWindowExW(0, kWindowClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
                                GetModuleHandleW(nullptr), nullptr);
    if(hwnd) {
        m_hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        m_notificationHandle = RegisterPowerSettingNotification(hwnd, &kAcDcPowerSourceGuid, DEVICE_NOTIFY_WINDOW_HANDLE);
    }
    if(!m_notificationHandle)
        TRC_WARN("charger notifications unavailable");

    // First reading only sets the baseline.
    refresh();
}

void PowerStatusController::refresh() {
    SYSTEM_POWER_STATUS status = {};
    if(!GetSystemPowerStatus(&status)) {
        setAvailable(false);
        return;
    }

    if(status.BatteryFlag != kUnknownValue)
        setAvailable((status.BatteryFlag & kNoBatteryFlag) == 0);

    mf::PowerSnapshot snapshot;
    if(status.ACLineStatus == 1)
        snapshot.source = mf::PowerSource::AC;
    else if(status.ACLineStatus == 0)
        snapshot.source = mf::PowerSource::Battery;
    snapshot.percent = status.BatteryLifePercent == kUnknownValue ? -1 : status.BatteryLifePercent;

    const std::optional<mf::ChargerAlert> alert = m_policy.update(snapshot);

    if(snapshot.source != mf::PowerSource::Unknown) {
        const bool plugged = snapshot.source == mf::PowerSource::AC;
        if(plugged != m_pluggedIn || snapshot.percent != m_percent) {
            m_pluggedIn = plugged;
            m_percent = snapshot.percent;
            m_bucket = mf::bucketLabel(mf::bucketOf(snapshot.percent));
            emit changed();
        }
    }

    if(alert) {
        TRC_INFO("charger {} at {}%", alert->event == mf::ChargerEvent::PluggedIn ? "plugged in" : "unplugged",
                 alert->percent);
        emit chargerChanged(alert->event == mf::ChargerEvent::PluggedIn);
    }
}

void PowerStatusController::setAvailable(bool available) {
    if(m_available == available)
        return;
    m_available = available;
    TRC_INFO("charger available -> {}", available);
    emit availabilityChanged();
}
