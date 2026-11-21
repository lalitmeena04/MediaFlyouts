#include "GlobalShortcutController.h"

#include <QCoreApplication>
#include <QKeySequence>

#include <Windows.h>

#include "Logging.h"
#include "Settings.h"

namespace {

struct Binding {
    const char* action;
    QString (Settings::*sequence)() const;
};

const Binding kBindings[] = {
    {"playPause", &Settings::shortcutPlayPause},
    {"next", &Settings::shortcutNext},
    {"toggleMute", &Settings::shortcutToggleMute},
    {"openFlyout", &Settings::shortcutOpenFlyout},
    {"toggleWidget", &Settings::shortcutToggleWidget},
    {"togglePeripheralsWidget", &Settings::shortcutTogglePeripheralsWidget},
    {"toggleFocusMode", &Settings::shortcutToggleFocusMode},
};

bool virtualKeyFor(int key, UINT* virtualKey) {
    if(key >= Qt::Key_A && key <= Qt::Key_Z) {
        *virtualKey = static_cast<UINT>(key);
        return true;
    }
    if(key >= Qt::Key_0 && key <= Qt::Key_9) {
        *virtualKey = static_cast<UINT>(key);
        return true;
    }
    if(key >= Qt::Key_F1 && key <= Qt::Key_F24) {
        *virtualKey = VK_F1 + static_cast<UINT>(key - Qt::Key_F1);
        return true;
    }

    switch(key) {
    case Qt::Key_Space: *virtualKey = VK_SPACE; return true;
    case Qt::Key_Tab: *virtualKey = VK_TAB; return true;
    case Qt::Key_Backspace: *virtualKey = VK_BACK; return true;
    case Qt::Key_Return:
    case Qt::Key_Enter: *virtualKey = VK_RETURN; return true;
    case Qt::Key_Insert: *virtualKey = VK_INSERT; return true;
    case Qt::Key_Delete: *virtualKey = VK_DELETE; return true;
    case Qt::Key_Home: *virtualKey = VK_HOME; return true;
    case Qt::Key_End: *virtualKey = VK_END; return true;
    case Qt::Key_PageUp: *virtualKey = VK_PRIOR; return true;
    case Qt::Key_PageDown: *virtualKey = VK_NEXT; return true;
    case Qt::Key_Left: *virtualKey = VK_LEFT; return true;
    case Qt::Key_Up: *virtualKey = VK_UP; return true;
    case Qt::Key_Right: *virtualKey = VK_RIGHT; return true;
    case Qt::Key_Down: *virtualKey = VK_DOWN; return true;
    default: return false;
    }
}

bool parseShortcut(const QString& text, UINT* modifiers, UINT* virtualKey) {
    const QKeySequence sequence = QKeySequence::fromString(text, QKeySequence::PortableText);
    if(sequence.count() != 1)
        return false;

    const int combined = sequence[0].toCombined();
    const int key = combined & ~static_cast<int>(Qt::KeyboardModifierMask);
    const Qt::KeyboardModifiers qtModifiers =
        static_cast<Qt::KeyboardModifiers>(combined & static_cast<int>(Qt::KeyboardModifierMask));
    UINT nativeModifiers = MOD_NOREPEAT;
    if(qtModifiers.testFlag(Qt::ControlModifier))
        nativeModifiers |= MOD_CONTROL;
    if(qtModifiers.testFlag(Qt::AltModifier))
        nativeModifiers |= MOD_ALT;
    if(qtModifiers.testFlag(Qt::ShiftModifier))
        nativeModifiers |= MOD_SHIFT;
    if(qtModifiers.testFlag(Qt::MetaModifier))
        nativeModifiers |= MOD_WIN;
    if(nativeModifiers == MOD_NOREPEAT || !virtualKeyFor(key, virtualKey))
        return false;
    *modifiers = nativeModifiers;
    return true;
}

} // namespace

GlobalShortcutController::GlobalShortcutController(Settings* settings, QObject* parent)
    : QObject(parent), m_settings(settings) {
    QCoreApplication::instance()->installNativeEventFilter(this);
    connect(m_settings, &Settings::changed, this, &GlobalShortcutController::updateBindings);
    updateBindings();
}

GlobalShortcutController::~GlobalShortcutController() {
    unregisterAll();
    QCoreApplication::instance()->removeNativeEventFilter(this);
}

QString GlobalShortcutController::status() const {
    return m_status;
}

void GlobalShortcutController::setCaptureMode(bool capturing) {
    if(m_captureMode == capturing)
        return;
    m_captureMode = capturing;
    updateBindings();
}

bool GlobalShortcutController::nativeEventFilter(const QByteArray&, void* message, qintptr* result) {
    const auto* msg = static_cast<const MSG*>(message);
    if(msg->message != WM_HOTKEY)
        return false;

    QString action;
    for(const Registration& registration : m_registrations) {
        if(registration.registered && registration.id == static_cast<int>(msg->wParam)) {
            action = registration.action;
            break;
        }
    }
    if(action.isEmpty())
        return false;

    if(result)
        *result = 0;
    emit actionTriggered(action);
    return true;
}

void GlobalShortcutController::updateBindings() {
    unregisterAll();
    if(m_captureMode) {
        setStatus(tr("Shortcuts are paused while assigning a key."));
        return;
    }

    QStringList failures;
    int registeredCount = 0;
    constexpr int bindingCount = static_cast<int>(sizeof(kBindings) / sizeof(kBindings[0]));
    for(int i = 0; i < bindingCount; ++i) {
        const Binding& binding = kBindings[i];
        const QString sequence = (m_settings->*(binding.sequence))();
        if(sequence.isEmpty())
            continue;

        Registration registration;
        registration.id = 0x4D00 + i;
        registration.action = QString::fromLatin1(binding.action);
        UINT modifiers = 0;
        UINT virtualKey = 0;
        if(!parseShortcut(sequence, &modifiers, &virtualKey)) {
            failures.append(tr("%1: unsupported shortcut").arg(sequence));
            TRC_WARN("shortcuts: unsupported binding '{}' for {}", sequence.toStdString(), binding.action);
            m_registrations.append(registration);
            continue;
        }

        if(!RegisterHotKey(nullptr, registration.id, modifiers, virtualKey)) {
            const DWORD error = GetLastError();
            failures.append(tr("%1: shortcut is already in use").arg(sequence));
            TRC_WARN("shortcuts: could not register '{}' for {} (error {})", sequence.toStdString(), binding.action,
                     error);
            m_registrations.append(registration);
            continue;
        }

        registration.registered = true;
        ++registeredCount;
        m_registrations.append(registration);
    }

    if(!failures.isEmpty())
        setStatus(failures.join(QStringLiteral("\n")));
    else
        setStatus(tr("%1 shortcut(s) active.").arg(registeredCount));
}

void GlobalShortcutController::unregisterAll() {
    for(const Registration& registration : m_registrations)
        if(registration.registered)
            UnregisterHotKey(nullptr, registration.id);
    m_registrations.clear();
}

void GlobalShortcutController::setStatus(const QString& status) {
    if(m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}
