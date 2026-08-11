#include "nativeflyoutsuppressor.h"

#include <windows.h>

namespace {

// Walks the shell's window tree to the volume OSD host, matching MediaFlyouts / gpkgpk/HideVolumeOSD:
//   XamlExplorerHostIslandWindow > DesktopWindowContentBridge("DesktopWindowXamlSource") > InputSite.WindowClass
// The OSD island is the one whose InputSite child has a non-empty rect.
HWND findOsdHost() {
    HWND island = nullptr;
    while((island = FindWindowExW(nullptr, island, L"XamlExplorerHostIslandWindow", nullptr)) != nullptr) {
        HWND bridge = FindWindowExW(island, nullptr, L"Windows.UI.Composition.DesktopWindowContentBridge",
                                    L"DesktopWindowXamlSource");
        if(!bridge)
            continue;
        HWND input = FindWindowExW(bridge, nullptr, L"Windows.UI.Input.InputSite.WindowClass", nullptr);
        if(!input)
            continue;
        ShowWindow(input, SW_RESTORE);
        RECT r{};
        if(GetWindowRect(input, &r) && (r.left || r.top || r.right || r.bottom))
            return island;
    }
    return nullptr;
}

} // namespace

NativeFlyoutSuppressor::NativeFlyoutSuppressor(QObject* parent) : QObject(parent) {}

NativeFlyoutSuppressor::~NativeFlyoutSuppressor() {
    restoreOsd();
}

void NativeFlyoutSuppressor::start() {
    parkOsd();
}

void NativeFlyoutSuppressor::arm() {
    if(!m_parked)
        parkOsd();
}

bool NativeFlyoutSuppressor::parkOsd() {
    HWND host = findOsdHost();
    if(!host)
        return false;

    m_osdHost = host;
    m_originalExStyle = GetWindowLongW(host, GWL_EXSTYLE);
    SetWindowLongW(host, GWL_EXSTYLE, m_originalExStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    SetWindowPos(host, nullptr, -99999, -99999, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(host, SW_MINIMIZE);
    m_parked = true;
    return true;
}

void NativeFlyoutSuppressor::restoreOsd() {
    if(!m_osdHost)
        return;

    HWND host = static_cast<HWND>(m_osdHost);
    SetWindowLongW(host, GWL_EXSTYLE, m_originalExStyle);
    SetWindowPos(host, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(host, SW_RESTORE);
    m_osdHost = nullptr;
    m_parked = false;
}
