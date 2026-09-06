#include "FullscreenDetector.h"

#include <cstdint>

#include <windows.h>

#include <shellapi.h>
#include <shlobj.h>
#include <tlhelp32.h>

#include <QGuiApplication>
#include <QScreen>

#include "Logging.h"

namespace {

// Hex pointer value for logging Win32 handles.
std::uintptr_t handleId(void* h) {
    return reinterpret_cast<std::uintptr_t>(h);
}

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

// True when secure screen is active input desktop.
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

// True if process has a 3D API loaded. This helps detect games.
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
        if(GetLastError() != ERROR_BAD_LENGTH) // Module list can change during snapshot.
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

// Get HMONITOR for screen index. QScreen::name() is not GDI device name.
// Map by screen center point.
HMONITOR monitorHandleForScreen(int index) {
    const QList<QScreen*> screens = QGuiApplication::screens();
    if(screens.isEmpty()) {
        TRC_WARN("monitorHandleForScreen: no screens available");
        return nullptr;
    }
    if(index < 0 || index >= screens.size()) {
        TRC_DEBUG("monitorHandleForScreen: index {} out of range (screenCount={}), clamping to 0", index,
                  screens.size());
        index = 0;
    }
    const QScreen* screen = screens.at(index);
    const qreal dpr = screen->devicePixelRatio();
    const QRect g = screen->geometry();
    const POINT center{static_cast<LONG>(qRound((g.left() + g.width() / 2.0) * dpr)),
                       static_cast<LONG>(qRound((g.top() + g.height() / 2.0) * dpr))};
    const HMONITOR handle = MonitorFromPoint(center, MONITOR_DEFAULTTONULL);
    TRC_DEBUG("monitorHandleForScreen: index={} name='{}' center=({},{}) -> HMONITOR={:#x}", index,
              screen->name().toStdString(), center.x, center.y, handleId(handle));
    return handle;
}

} // namespace

FullscreenDetector::FullscreenDetector(QObject* parent) : QObject(parent) {}

bool FullscreenDetector::isLocked() const {
    const bool locked = isWorkstationLocked();
    TRC_DEBUG("isLocked -> {}", locked);
    return locked;
}

bool FullscreenDetector::shouldBlock(bool disableOnFullscreen, int monitor) const {
    const bool locked = isLocked();
    const bool fullscreen = disableOnFullscreen && isGraphicsFullscreen(monitor);
    const bool blocked = locked || fullscreen;
    TRC_DEBUG("shouldBlock(disableOnFullscreen={}, monitor={}): locked={} fullscreen={} -> blocked={}",
              disableOnFullscreen, monitor, locked, fullscreen, blocked);
    return blocked;
}

bool FullscreenDetector::isFullscreen(int monitor) const {
    HWND fg = GetForegroundWindow();
    TRC_DEBUG("isFullscreen(monitor={}): foreground HWND={:#x}", monitor, handleId(fg));

    if(!fg) {
        TRC_DEBUG("isFullscreen: no foreground window -> not fullscreen");
        return false;
    }
    if(isStartMenuWindow(fg)) {
        TRC_DEBUG("isFullscreen: foreground is the Start menu -> not fullscreen");
        return false;
    }

    // Desktop/taskbar is not a game. Rule it out before notification query.
    // Query can stay stuck after fullscreen game exits.
    wchar_t cls[64] = {};
    GetClassNameW(fg, cls, 64);
    const std::string className = QString::fromWCharArray(cls).toStdString();
    TRC_DEBUG("isFullscreen: foreground class='{}'", className);
    if(wcscmp(cls, L"Progman") == 0 || wcscmp(cls, L"WorkerW") == 0 || wcscmp(cls, L"Shell_TrayWnd") == 0) {
        TRC_DEBUG("isFullscreen: foreground is the desktop/taskbar shell -> not fullscreen");
        return false;
    }

    // Fullscreen app blocks only its own monitor. Other monitors stay visible.
    if(monitor >= 0) {
        const HMONITOR target = monitorHandleForScreen(monitor);
        const HMONITOR fgMonitor = MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST);
        TRC_DEBUG("isFullscreen: monitor gate: targetHMONITOR={:#x} foregroundHMONITOR={:#x}", handleId(target),
                  handleId(fgMonitor));
        if(target && fgMonitor != target) {
            TRC_DEBUG("isFullscreen: foreground window is on a different monitor -> not fullscreen here");
            return false;
        }
        if(!target)
            TRC_WARN("isFullscreen: could not resolve target HMONITOR for monitor index {} - skipping monitor gate",
                     monitor);
    }

    // Exclusive D3D or presentation fullscreen. QUNS_BUSY is not fullscreen.
    // It can stay stuck after a game exits. Borderless games use check below.
    QUERY_USER_NOTIFICATION_STATE state = QUNS_ACCEPTS_NOTIFICATIONS;
    if(SUCCEEDED(SHQueryUserNotificationState(&state))) {
        TRC_DEBUG("isFullscreen: SHQueryUserNotificationState={}", static_cast<int>(state));
        if(state == QUNS_RUNNING_D3D_FULL_SCREEN || state == QUNS_PRESENTATION_MODE) {
            TRC_WARN("isFullscreen: notification state {} indicates exclusive/presentation fullscreen -> fullscreen",
                     static_cast<int>(state));
            return true;
        }
    } else {
        TRC_DEBUG("isFullscreen: SHQueryUserNotificationState query failed");
    }

    // Borderless fullscreen. Foreground window covers whole monitor.
    RECT wr{};
    if(!GetWindowRect(fg, &wr)) {
        TRC_DEBUG("isFullscreen: GetWindowRect failed -> not fullscreen");
        return false;
    }

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    if(!GetMonitorInfoW(MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST), &mi)) {
        TRC_DEBUG("isFullscreen: GetMonitorInfo failed -> not fullscreen");
        return false;
    }

    const RECT m = mi.rcMonitor;
    TRC_DEBUG("isFullscreen: windowRect=({},{},{},{}) monitorRect=({},{},{},{})", wr.left, wr.top, wr.right, wr.bottom,
              m.left, m.top, m.right, m.bottom);
    if(!(wr.left <= m.left && wr.top <= m.top && wr.right >= m.right && wr.bottom >= m.bottom)) {
        TRC_DEBUG("isFullscreen: window does not cover the whole monitor -> not fullscreen");
        return false;
    }

    TRC_DEBUG("isFullscreen: foreground window covers the whole monitor -> fullscreen");
    return true;
}

bool FullscreenDetector::isGraphicsFullscreen(int monitor) const {
    if(!isFullscreen(monitor))
        return false;

    QUERY_USER_NOTIFICATION_STATE state = QUNS_ACCEPTS_NOTIFICATIONS;
    if(SUCCEEDED(SHQueryUserNotificationState(&state)) && state == QUNS_RUNNING_D3D_FULL_SCREEN) {
        TRC_WARN("isGraphicsFullscreen: notification state indicates exclusive D3D fullscreen -> fullscreen");
        return true;
    }

    HWND fg = GetForegroundWindow();
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    const bool graphics = pid != 0 && hasGraphicsApiLoaded(pid);
    TRC_DEBUG("isGraphicsFullscreen: fullscreen app pid={} graphicsApiLoaded={} -> fullscreen={}", pid, graphics,
              graphics);
    return graphics;
}
