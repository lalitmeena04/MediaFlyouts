#include "WindowEffects.h"

#include <QWindow>

#include <windows.h>

#include <dwmapi.h>

namespace {
// DWM attributes (available on Windows 11 build 22000+); harmless no-ops elsewhere.
constexpr DWORD kUseImmersiveDarkMode = 20;
constexpr DWORD kSystemBackdropType = 38;
constexpr int kBackdropNone = 1;
constexpr int kBackdropMica = 2;      // mica (main window)
constexpr int kBackdropTransient = 3; // acrylic

HWND hwndOf(QWindow* window) {
    return window ? reinterpret_cast<HWND>(window->winId()) : nullptr;
}
} // namespace

WindowEffects::WindowEffects(QObject* parent) : QObject(parent) {}

void WindowEffects::applyBackdrop(QWindow* window, int mode) {
    HWND hwnd = hwndOf(window);
    if(!hwnd)
        return;
    const bool enable = mode != 0;
    // extend the frame so the backdrop fills the whole window
    MARGINS margins = enable ? MARGINS{-1, -1, -1, -1} : MARGINS{0, 0, 0, 0};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    int backdrop = kBackdropNone;
    if(mode == 1)
        backdrop = kBackdropTransient;
    else if(mode == 2)
        backdrop = kBackdropMica;
    DwmSetWindowAttribute(hwnd, kSystemBackdropType, &backdrop, sizeof(backdrop));
}

void WindowEffects::setDarkFrame(QWindow* window, bool dark) {
    HWND hwnd = hwndOf(window);
    if(!hwnd)
        return;
    BOOL value = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, kUseImmersiveDarkMode, &value, sizeof(value));
}
