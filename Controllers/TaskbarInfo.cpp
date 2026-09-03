#include "TaskbarInfo.h"

#include <cstdint>

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <windows.h>

#include "Logging.h"

namespace {
// Convert a native window rect (physical pixels) into Qt logical coordinates.
QRect toLogical(HWND hwnd) {
    if(!hwnd)
        return QRect();
    RECT r{};
    if(!GetWindowRect(hwnd, &r))
        return QRect();

    qreal dpr = 1.0;
    if(const QScreen* s = QGuiApplication::primaryScreen())
        dpr = s->devicePixelRatio();
    if(dpr <= 0)
        dpr = 1.0;

    return QRect(qRound(r.left / dpr), qRound(r.top / dpr), qRound((r.right - r.left) / dpr),
                 qRound((r.bottom - r.top) / dpr));
}

TaskbarInfo* g_taskbarInfo = nullptr;
HWINEVENTHOOK g_winEventHook = nullptr;

// Fires when the foreground window or a window's minimize state changes; the shell re-tops the
// taskbar on these, which can push our always-on-top widgets behind it, so re-pin immediately.
void CALLBACK winEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    TRC_DEBUG("winEventProc: event={:#x} hwnd={:#x}", event, reinterpret_cast<std::uintptr_t>(hwnd));
    if(g_taskbarInfo)
        g_taskbarInfo->handleShellEvent(event);
}
} // namespace

TaskbarInfo::TaskbarInfo(QObject* parent) : QObject(parent) {
    g_taskbarInfo = this;
    // keep re-pinning across the ~300ms minimize/restore animation, during which the taskbar
    // repeatedly re-tops itself and would otherwise cover the widgets until the next 500ms poll
    m_repinBurst.setInterval(30);
    connect(&m_repinBurst, &QTimer::timeout, this, [this] {
        repinTracked();
        if(--m_repinBurstTicks <= 0)
            m_repinBurst.stop();
    });
    // OUTOFCONTEXT delivers the callback on this (the GUI) thread, so touching QWindow is safe
    g_winEventHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_MINIMIZEEND, nullptr, winEventProc, 0, 0,
                                     WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    TRC_INFO("TaskbarInfo: WinEvent hook installed={}", g_winEventHook != nullptr);
}

TaskbarInfo::~TaskbarInfo() {
    if(g_winEventHook) {
        UnhookWinEvent(g_winEventHook);
        g_winEventHook = nullptr;
    }
    g_taskbarInfo = nullptr;
}


QRect TaskbarInfo::taskbarGeometry() const {
    return toLogical(FindWindowW(L"Shell_TrayWnd", nullptr));
}

QRect TaskbarInfo::trayGeometry() const {
    HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if(!taskbar)
        return QRect();
    return toLogical(FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr));
}

QPoint TaskbarInfo::widgetPosition(int monitor, int side, int offset, int width, int height) const {
    const QList<QScreen*> screens = QGuiApplication::screens();
    if(screens.isEmpty())
        return QPoint(0, 0);
    if(monitor < 0 || monitor >= screens.size())
        monitor = 0;
    const QScreen* screen = screens.at(monitor);

    // QScreen geometry is already logical and per-monitor DPI aware
    const QRect full = screen->geometry();
    const QRect avail = screen->availableGeometry();

    // taskbar strip height for a bottom taskbar; fall back to 40px if none is docked here
    int taskbarHeight = full.bottom() - avail.bottom();
    int taskbarTop;
    if(taskbarHeight > 0) {
        taskbarTop = avail.bottom() + 1;
    } else {
        taskbarHeight = 40;
        taskbarTop = full.bottom() + 1 - taskbarHeight;
    }
    const int y = taskbarTop + qMax(0, (taskbarHeight - height) / 2);

    int x;
    if(side == 0) {
        // left edge; offset nudges it right
        x = full.left() + 8 + qMax(0, offset);
    } else if(side == 1) {
        // centered on the monitor; signed offset nudges it left or right
        x = full.left() + (full.width() - width) / 2 + offset;
    } else {
        int rightEdge = full.right() + 1 - 200;
        // use the precise tray boundary only on the monitor that hosts the primary tray
        if(screen == QGuiApplication::primaryScreen()) {
            const QRect tray = trayGeometry();
            if(!tray.isEmpty())
                rightEdge = tray.left();
        }
        x = rightEdge - width - 8 - qMax(0, offset);
    }
    return QPoint(x, y);
}

QRect TaskbarInfo::monitorGeometry(int monitor) const {
    const QList<QScreen*> screens = QGuiApplication::screens();
    if(screens.isEmpty())
        return QRect();
    if(monitor < 0 || monitor >= screens.size())
        monitor = 0;
    return screens.at(monitor)->geometry();
}

void TaskbarInfo::pinAboveTaskbar(QWindow* window) const {
    if(!window)
        return;
    if(!m_pinned.contains(window))
        m_pinned.append(window);
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if(!hwnd)
        return;
    // move to the top of the topmost band (above the taskbar) without stealing focus
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void TaskbarInfo::repinTracked() const {
    int repinned = 0;
    for(auto it = m_pinned.begin(); it != m_pinned.end();) {
        QWindow* window = *it;
        if(!window) {
            it = m_pinned.erase(it);
            continue;
        }
        if(window->isVisible()) {
            if(HWND hwnd = reinterpret_cast<HWND>(window->winId())) {
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                ++repinned;
            }
        }
        ++it;
    }
    TRC_DEBUG("repinTracked: tracked={} repinned={}", m_pinned.size(), repinned);
}

void TaskbarInfo::handleShellEvent(unsigned long event) {
    repinTracked();
    if(event == EVENT_SYSTEM_MINIMIZESTART || event == EVENT_SYSTEM_MINIMIZEEND || event == EVENT_SYSTEM_FOREGROUND) {
        m_repinBurstTicks = 15; // ~450ms of coverage over the animation
        m_repinBurst.start();
    }
}
