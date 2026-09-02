#include "FullscreenDetector.h"

#include <windows.h>

#include <shellapi.h>
#include <shlobj.h>

namespace {

bool isStartMenuWindow(HWND window) {
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if(!process)
        return false;

    wchar_t path[MAX_PATH] = {};
    DWORD pathSize = MAX_PATH;
    const bool hasPath = QueryFullProcessImageNameW(process, 0, path, &pathSize);
    CloseHandle(process);
    if(!hasPath)
        return false;

    const wchar_t* executable = wcsrchr(path, L'\\');
    executable = executable ? executable + 1 : path;
    return lstrcmpiW(executable, L"StartMenuExperienceHost.exe") == 0;
}

// true when the secure / lock screen is the active input desktop
bool isWorkstationLocked() {
    HDESK desk = OpenInputDesktop(0, FALSE, DESKTOP_READOBJECTS);
    if(!desk)
        return true;
    wchar_t name[128] = {};
    DWORD needed = 0;
    const bool ok = GetUserObjectInformationW(desk, UOI_NAME, name, sizeof(name), &needed);
    CloseDesktop(desk);
    return ok && lstrcmpiW(name, L"Default") != 0;
}

} // namespace

FullscreenDetector::FullscreenDetector(QObject* parent) : QObject(parent) {}

bool FullscreenDetector::isLocked() const {
    return isWorkstationLocked();
}

bool FullscreenDetector::shouldBlock(bool disableOnFullscreen) const {
    return isLocked() || (disableOnFullscreen && isFullscreen());
}

bool FullscreenDetector::isFullscreen() const {
    HWND fg = GetForegroundWindow();
    if(fg && isStartMenuWindow(fg))
        return false;

    // exclusive-D3D / presentation / generic fullscreen — the states that cause a game to minimize
    QUERY_USER_NOTIFICATION_STATE state = QUNS_ACCEPTS_NOTIFICATIONS;
    if(SUCCEEDED(SHQueryUserNotificationState(&state))) {
        if(state == QUNS_RUNNING_D3D_FULL_SCREEN || state == QUNS_PRESENTATION_MODE || state == QUNS_BUSY)
            return true;
    }

    // borderless fullscreen: foreground window covers its whole monitor and isn't the shell
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
