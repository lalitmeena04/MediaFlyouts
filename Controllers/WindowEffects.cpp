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

// Undocumented composition-attribute API — the reliable way to get real acrylic
// blur behind a *frameless* window (the DWM system backdrop only works on framed
// windows, so it renders as broken transparency on our tool-window flyouts).
enum AccentState {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
};
struct AccentPolicy {
        AccentState state;
        DWORD flags;
        DWORD gradientColor; // 0xAABBGGRR
        DWORD animationId;
};
struct WindowCompositionAttributeData {
        DWORD attribute; // WCA_ACCENT_POLICY = 19
        void* data;
        SIZE_T size;
};
using SetWindowCompositionAttributeFn = BOOL(WINAPI*)(HWND, WindowCompositionAttributeData*);

SetWindowCompositionAttributeFn compositionAttributeFn() {
    static SetWindowCompositionAttributeFn fn = [] {
        HMODULE user32 = GetModuleHandleW(L"user32.dll");
        return user32 ? reinterpret_cast<SetWindowCompositionAttributeFn>(
                            GetProcAddress(user32, "SetWindowCompositionAttribute"))
                      : nullptr;
    }();
    return fn;
}

HWND hwndOf(QWindow* window) {
    return window ? reinterpret_cast<HWND>(window->winId()) : nullptr;
}

bool isFramed(HWND hwnd) {
    return (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION;
}

void setCompositionAcrylic(HWND hwnd, bool enable) {
    SetWindowCompositionAttributeFn fn = compositionAttributeFn();
    if(!fn)
        return;
    AccentPolicy policy{};
    // a subtle dark tint so the blurred content stays legible; disabled = solid
    policy.state = enable ? ACCENT_ENABLE_ACRYLICBLURBEHIND : ACCENT_DISABLED;
    policy.gradientColor = enable ? 0x33000000 : 0;
    WindowCompositionAttributeData data{19, &policy, sizeof(policy)};
    fn(hwnd, &data);
}
} // namespace

WindowEffects::WindowEffects(QObject* parent) : QObject(parent) {}

void WindowEffects::applyBackdrop(QWindow* window, int mode) {
    HWND hwnd = hwndOf(window);
    if(!hwnd)
        return;

    // frameless flyouts: use the composition acrylic which actually renders on popups
    if(!isFramed(hwnd)) {
        setCompositionAcrylic(hwnd, mode != 0);
        return;
    }

    // framed windows (dashboard / mixer / onboarding): use the DWM system backdrop
    const bool enable = mode != 0;
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
