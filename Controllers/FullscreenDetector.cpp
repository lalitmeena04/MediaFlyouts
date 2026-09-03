#include "FullscreenDetector.h"

#include <windows.h>

#include <shellapi.h>
#include <shlobj.h>
#include <tlhelp32.h>

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

// true if the process has a 3D rendering API loaded, the signal used to tell a game apart
// from a fullscreen browser, video player, or presentation slideshow
bool hasGraphicsApiLoaded(DWORD pid) {
    static const wchar_t* kGraphicsModules[] = {
        L"d3d9.dll", L"d3d10.dll", L"d3d10_1.dll", L"d3d11.dll", L"d3d12.dll",
        L"dxgi.dll", L"opengl32.dll", L"vulkan-1.dll"
    };

    HANDLE snap = INVALID_HANDLE_VALUE;
    for(int attempt = 0; attempt < 5; ++attempt) {
        snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if(snap != INVALID_HANDLE_VALUE)
            break;
        if(GetLastError() != ERROR_BAD_LENGTH) // transient while the target's module list is changing
            return false;
    }
    if(snap == INVALID_HANDLE_VALUE)
        return false;

    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    bool found = false;
    if(Module32FirstW(snap, &me)) {
        do {
            for(const wchar_t* name : kGraphicsModules) {
                if(_wcsicmp(me.szModule, name) == 0) {
                    found = true;
                    break;
                }
            }
        } while(!found && Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return found;
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

    if(!fg)
        return false;
    if(isStartMenuWindow(fg))
        return false;

    // desktop/taskbar in the foreground (no window open) is never a game; rule it out before the
    // notification query, which can stay stuck on QUNS_BUSY after a fullscreen game exits (until reboot)
    wchar_t cls[64] = {};
    GetClassNameW(fg, cls, 64);
    if(wcscmp(cls, L"Progman") == 0 || wcscmp(cls, L"WorkerW") == 0 || wcscmp(cls, L"Shell_TrayWnd") == 0)
        return false;

    // exclusive-D3D / presentation / generic fullscreen — the states that cause a game to minimize
    QUERY_USER_NOTIFICATION_STATE state = QUNS_ACCEPTS_NOTIFICATIONS;
    if(SUCCEEDED(SHQueryUserNotificationState(&state))) {
        if(state == QUNS_RUNNING_D3D_FULL_SCREEN || state == QUNS_PRESENTATION_MODE || state == QUNS_BUSY)
            return true;
    }

    // borderless fullscreen: foreground window covers its whole monitor and isn't the shell
    RECT wr{};
    if(!GetWindowRect(fg, &wr))
        return false;

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    if(!GetMonitorInfoW(MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST), &mi))
        return false;

    const RECT m = mi.rcMonitor;
    if(!(wr.left <= m.left && wr.top <= m.top && wr.right >= m.right && wr.bottom >= m.bottom))
        return false;

    // borderless fullscreen alone also matches fullscreen browsers/video players, so require a
    // 3D graphics API to be loaded before treating this as a game
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    return pid != 0 && hasGraphicsApiLoaded(pid);
}
