#include "nativeflyoutsuppressor.h"

#include <atomic>

#include <QDateTime>

#include <windows.h>

namespace {

HWINEVENTHOOK g_hook = nullptr;
std::atomic<qint64> g_armedUntilMs{0};

// the modern volume/brightness OSD is a top-level "NativeHWNDHost" containing a "DirectUIHWND"
bool isNativeOsdHost(HWND hwnd) {
    wchar_t cls[64] = {};
    if(GetClassNameW(hwnd, cls, 64) == 0)
        return false;
    if(wcscmp(cls, L"NativeHWNDHost") != 0)
        return false;
    return FindWindowExW(hwnd, nullptr, L"DirectUIHWND", nullptr) != nullptr;
}

void CALLBACK winEventProc(HWINEVENTHOOK, DWORD, HWND hwnd, LONG idObject, LONG idChild, DWORD, DWORD) {
    if(idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
        return;
    if(QDateTime::currentMSecsSinceEpoch() > g_armedUntilMs.load())
        return;
    if(isNativeOsdHost(hwnd))
        ShowWindow(hwnd, SW_HIDE);
}

BOOL CALLBACK enumHideProc(HWND hwnd, LPARAM) {
    if(IsWindowVisible(hwnd) && isNativeOsdHost(hwnd))
        ShowWindow(hwnd, SW_HIDE);
    return TRUE;
}

} // namespace

NativeFlyoutSuppressor::NativeFlyoutSuppressor(QObject* parent) : QObject(parent) {}

NativeFlyoutSuppressor::~NativeFlyoutSuppressor() {
    if(g_hook) {
        UnhookWinEvent(g_hook);
        g_hook = nullptr;
    }
}

void NativeFlyoutSuppressor::start() {
    g_hook = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr, winEventProc, 0, 0,
                             WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
}

void NativeFlyoutSuppressor::arm() {
    g_armedUntilMs.store(QDateTime::currentMSecsSinceEpoch() + 1000);
    EnumWindows(enumHideProc, 0);
}
