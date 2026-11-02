#include "LockKeysController.h"

#include <windows.h>

#include "Logging.h"

namespace {

LockKeysController* g_instance = nullptr;
HHOOK g_hook = nullptr;

// Hook install thread runs callback. Qt main thread pumps messages.
LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode == HC_ACTION && wParam == WM_KEYUP) {
        const DWORD vk = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode;
        if((vk == VK_CAPITAL || vk == VK_NUMLOCK || vk == VK_SCROLL || vk == VK_INSERT) && g_instance)
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
    m_available = g_hook != nullptr;
    if(m_available)
        TRC_INFO("lock keys: keyboard hook installed");
    else
        TRC_WARN("lock keys: keyboard hook failed (error={})", GetLastError());
    emit availabilityChanged();
}

bool LockKeysController::available() const {
    return m_available;
}

QString LockKeysController::keyName() const {
    return m_keyName;
}

bool LockKeysController::on() const {
    return m_on;
}

int LockKeysController::keyType() const {
    return m_keyType;
}

void LockKeysController::handleToggle(int vk) {
    switch(vk) {
    case VK_CAPITAL:
        m_keyName = tr("Caps Lock");
        m_keyType = 0;
        break;
    case VK_NUMLOCK:
        m_keyName = tr("Num Lock");
        m_keyType = 1;
        break;
    case VK_SCROLL:
        m_keyName = tr("Scroll Lock");
        m_keyType = 2;
        break;
    case VK_INSERT:
        m_keyName = tr("Insert");
        m_keyType = 3;
        break;
    default:
        return;
    }
    m_on = (GetKeyState(vk) & 1) != 0;
    emit changed();
    emit lockKeyToggled();
}
