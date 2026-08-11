#include "mediakeycontroller.h"

#include <windows.h>

namespace {

MediaKeyController* g_instance = nullptr;
HHOOK g_hook = nullptr;

LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        const DWORD vk = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode;
        if((vk == VK_MEDIA_PLAY_PAUSE || vk == VK_MEDIA_NEXT_TRACK || vk == VK_MEDIA_PREV_TRACK ||
            vk == VK_MEDIA_STOP) &&
           g_instance)
            g_instance->notifyPressed();
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
}

void MediaKeyController::notifyPressed() {
    emit mediaKeyPressed();
}
