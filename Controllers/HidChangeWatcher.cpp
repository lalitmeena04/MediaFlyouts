#include "HidChangeWatcher.h"

#include <windows.h>

#include <dbt.h>

#include "Logging.h"

namespace {

// GUID_DEVINTERFACE_HID.
// Hardcoded to avoid hidclass.h and hidpi.h conflict.
constexpr GUID kHidInterfaceGuid = {
    0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};

constexpr wchar_t kWindowClassName[] = L"MediaFlyoutsHidChangeWatcher";

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if(msg == WM_DEVICECHANGE && (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)) {
        auto* self = reinterpret_cast<HidChangeWatcher*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if(self)
            emit self->deviceChanged();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

HidChangeWatcher::HidChangeWatcher(QObject* parent) : QObject(parent) {}

HidChangeWatcher::~HidChangeWatcher() {
    if(m_notificationHandle)
        UnregisterDeviceNotification(m_notificationHandle);
    if(m_hwnd)
        DestroyWindow(reinterpret_cast<HWND>(m_hwnd));
}

void HidChangeWatcher::start() {
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
    if(!hwnd) {
        TRC_WARN("hid watcher: message window creation failed (error={})", GetLastError());
        return;
    }
    m_hwnd = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    DEV_BROADCAST_DEVICEINTERFACE_W filter = {};
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = kHidInterfaceGuid;

    m_notificationHandle = RegisterDeviceNotificationW(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if(m_notificationHandle)
        TRC_INFO("hid watcher: device notifications registered");
    else
        TRC_WARN("hid watcher: RegisterDeviceNotification failed (error={})", GetLastError());
}
