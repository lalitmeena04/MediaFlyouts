#include "TaskbarInfo.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <windows.h>

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
} // namespace

TaskbarInfo::TaskbarInfo(QObject* parent) : QObject(parent) {}

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
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if(!hwnd)
        return;
    // keep the widget out of screenshots / the snip tool overlay grab
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif
    DWORD affinity = 0;
    if(!GetWindowDisplayAffinity(hwnd, &affinity) || affinity != WDA_EXCLUDEFROMCAPTURE)
        SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
    // move to the top of the topmost band (above the taskbar) without stealing focus
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}
