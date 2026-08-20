#include "WindowEffects.h"

#include <QWindow>

#include <windows.h>

#include <dwmapi.h>

namespace {
// DWM immersive dark mode (Windows 10 20H1+); harmless no-op elsewhere.
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
