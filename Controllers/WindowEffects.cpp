#include "WindowEffects.h"

#include <QWindow>

#include <windows.h>

#include <dwmapi.h>

namespace {
// DWM dark mode for Windows 10 20H1 and later.
constexpr DWORD kUseImmersiveDarkMode = 20;

HWND hwndOf(QWindow* window) {
    return window ? reinterpret_cast<HWND>(window->winId()) : nullptr;
}
} // namespace

WindowEffects::WindowEffects(QObject* parent) : QObject(parent) {}

void WindowEffects::setDarkFrame(QWindow* window, bool dark) {
    HWND hwnd = hwndOf(window);
    if(!hwnd)
        return;
    BOOL value = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, kUseImmersiveDarkMode, &value, sizeof(value));
}

void WindowEffects::pinTopmost(QWindow* window) {
    HWND hwnd = hwndOf(window);
    if(!hwnd)
        return;

    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}
