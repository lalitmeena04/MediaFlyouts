#include "nativeflyoutsuppressor.h"

#include <atomic>

#include <QDateTime>
#include <QString>

#include <windows.h>

namespace {

HWINEVENTHOOK g_hookShow = nullptr;
HWINEVENTHOOK g_hookUncloak = nullptr;
std::atomic<qint64> g_armedUntilMs{0};
DWORD g_shellPid = 0;

// cheap check: the OSD host belongs to the shell (explorer) process
bool isExplorer(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid != 0 && pid == g_shellPid;
}

// the volume OSD is a small, explorer-owned XAML island on Win11, or the legacy NativeHWNDHost on Win10
bool isNativeVolumeOsd(HWND hwnd) {
    wchar_t cls[80] = {};
    if(GetClassNameW(hwnd, cls, 80) == 0)
        return false;

    if(wcscmp(cls, L"XamlExplorerHostIslandWindow") == 0) {
        RECT r{};
        GetWindowRect(hwnd, &r);
        const int w = r.right - r.left;
        const int h = r.bottom - r.top;
        if(w <= 0 || h <= 0 || w > 520 || h > 220)
            return false;
        return isExplorer(hwnd);
    }

    if(wcscmp(cls, L"NativeHWNDHost") == 0)
        return FindWindowExW(hwnd, nullptr, L"DirectUIHWND", nullptr) != nullptr;

    return false;
}

void hideIfOsd(HWND hwnd) {
    if(isNativeVolumeOsd(hwnd))
        ShowWindow(hwnd, SW_HIDE);
}

void CALLBACK winEventProc(HWINEVENTHOOK, DWORD, HWND hwnd, LONG idObject, LONG idChild, DWORD, DWORD) {
    if(idObject != OBJID_WINDOW || idChild != CHILDID_SELF || !hwnd)
        return;
    if(QDateTime::currentMSecsSinceEpoch() > g_armedUntilMs.load())
        return;
    hideIfOsd(hwnd);
}

BOOL CALLBACK enumHideProc(HWND hwnd, LPARAM) {
    if(IsWindowVisible(hwnd))
        hideIfOsd(hwnd);
    return TRUE;
}

} // namespace

NativeFlyoutSuppressor::NativeFlyoutSuppressor(QObject* parent) : QObject(parent) {}

NativeFlyoutSuppressor::~NativeFlyoutSuppressor() {
    if(g_hookShow) {
        UnhookWinEvent(g_hookShow);
        g_hookShow = nullptr;
    }
    if(g_hookUncloak) {
        UnhookWinEvent(g_hookUncloak);
        g_hookUncloak = nullptr;
    }
}

void NativeFlyoutSuppressor::start() {
    GetWindowThreadProcessId(GetShellWindow(), &g_shellPid);
    const DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    g_hookShow = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr, winEventProc, 0, 0, flags);
    g_hookUncloak = SetWinEventHook(EVENT_OBJECT_UNCLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, winEventProc, 0, 0, flags);
}

void NativeFlyoutSuppressor::arm() {
    g_armedUntilMs.store(QDateTime::currentMSecsSinceEpoch() + 1200);
    EnumWindows(enumHideProc, 0);
}
