#include "NativeFlyoutSuppressor.h"

#include <windows.h>

#include "Logging.h"

namespace {

// Find volume OSD host in shell window tree. Move it off-screen.
// Tree: XamlExplorerHostIslandWindow > DesktopWindowContentBridge > InputSite.WindowClass.
// OSD island has an InputSite child with a non-empty rect.
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

void NativeFlyoutSuppressor::restore() {
    restoreOsd();
}

bool NativeFlyoutSuppressor::parkOsd() {
    HWND host = findOsdHost();
    if(!host) {
        // Shell creates the OSD lazily. Later arm() calls retry.
        TRC_INFO("native osd: host window not found yet");
        return false;
    }

    m_osdHost = host;
    m_originalExStyle = GetWindowLongW(host, GWL_EXSTYLE);
    SetWindowLongW(host, GWL_EXSTYLE, m_originalExStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    SetWindowPos(host, nullptr, -99999, -99999, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(host, SW_MINIMIZE);
    m_parked = true;
    TRC_INFO("native osd: parked");
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
    TRC_INFO("native osd: restored");
}
