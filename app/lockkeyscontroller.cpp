#include "lockkeyscontroller.h"

#include <windows.h>

namespace {

LockKeysController* g_instance = nullptr;
HHOOK g_hook = nullptr;

// runs on the thread that installed the hook (Qt main thread pumps the messages)
LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode == HC_ACTION && wParam == WM_KEYUP) {
        const DWORD vk = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode;
        if((vk == VK_CAPITAL || vk == VK_NUMLOCK || vk == VK_SCROLL) && g_instance)
            g_instance->handleToggle(static_cast<int>(vk));
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace

LockKeysController::LockKeysController(QObject* parent) : QObject(parent) {
    g_instance = this;
}

LockKeysController::~LockKeysController() {
    if(g_hook) {
        UnhookWindowsHookEx(g_hook);
        g_hook = nullptr;
    }
    if(g_instance == this)
        g_instance = nullptr;
}

void LockKeysController::start() {
    g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, keyboardProc, GetModuleHandleW(nullptr), 0);
}

QString LockKeysController::keyName() const {
    return m_keyName;
}

bool LockKeysController::on() const {
    return m_on;
}

void LockKeysController::handleToggle(int vk) {
    switch(vk) {
    case VK_CAPITAL:
        m_keyName = QStringLiteral("Caps Lock");
        break;
    case VK_NUMLOCK:
        m_keyName = QStringLiteral("Num Lock");
        break;
    case VK_SCROLL:
        m_keyName = QStringLiteral("Scroll Lock");
        break;
    default:
        return;
    }
    m_on = (GetKeyState(vk) & 1) != 0;
    emit changed();
    emit lockKeyToggled();
}
