#include "fullscreendetector.h"

#include <windows.h>

#include <shellapi.h>
#include <shlobj.h>

FullscreenDetector::FullscreenDetector(QObject* parent) : QObject(parent) {}

bool FullscreenDetector::isActive() const {
    // exclusive-D3D / presentation / generic fullscreen — the states that cause a game to minimize
    QUERY_USER_NOTIFICATION_STATE state = QUNS_ACCEPTS_NOTIFICATIONS;
    if(SUCCEEDED(SHQueryUserNotificationState(&state))) {
        if(state == QUNS_RUNNING_D3D_FULL_SCREEN || state == QUNS_PRESENTATION_MODE || state == QUNS_BUSY)
            return true;
    }

    // borderless fullscreen: foreground window covers its whole monitor and isn't the shell
    HWND fg = GetForegroundWindow();
    if(!fg)
        return false;

    wchar_t cls[64] = {};
    GetClassNameW(fg, cls, 64);
    if(wcscmp(cls, L"Progman") == 0 || wcscmp(cls, L"WorkerW") == 0 || wcscmp(cls, L"Shell_TrayWnd") == 0)
        return false;

    RECT wr{};
    if(!GetWindowRect(fg, &wr))
        return false;

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    if(!GetMonitorInfoW(MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST), &mi))
        return false;

    const RECT m = mi.rcMonitor;
    return wr.left <= m.left && wr.top <= m.top && wr.right >= m.right && wr.bottom >= m.bottom;
}
