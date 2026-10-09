#include "MediaKeyController.h"

#include <windows.h>

#include "Logging.h"

namespace {

MediaKeyController* g_instance = nullptr;
HHOOK g_hook = nullptr;

LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode == HC_ACTION && wParam == WM_KEYDOWN && g_instance) {
        const DWORD vk = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode;
        if(vk == VK_MEDIA_PLAY_PAUSE || vk == VK_MEDIA_NEXT_TRACK || vk == VK_MEDIA_PREV_TRACK || vk == VK_MEDIA_STOP)
            g_instance->notifyPressed();
        else if(vk == VK_VOLUME_UP || vk == VK_VOLUME_DOWN || vk == VK_VOLUME_MUTE)
            g_instance->notifyVolumePressed();
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace

MediaKeyController::MediaKeyController(QObject* parent) : QObject(parent) {
    g_instance = this;
}

MediaKeyController::~MediaKeyController() {
    if(g_hook) {
        UnhookWindowsHookEx(g_hook);
        g_hook = nullptr;
    }
    if(g_instance == this)
        g_instance = nullptr;
}

void MediaKeyController::start() {
    g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, keyboardProc, GetModuleHandleW(nullptr), 0);
    m_available = g_hook != nullptr;
    if(m_available)
        TRC_INFO("media keys: keyboard hook installed");
    else
        TRC_WARN("media keys: keyboard hook failed (error={})", GetLastError());
    emit availabilityChanged();
}

bool MediaKeyController::available() const {
    return m_available;
}

void MediaKeyController::notifyPressed() {
    emit mediaKeyPressed();
}

void MediaKeyController::notifyVolumePressed() {
    emit volumeKeyPressed();
}
