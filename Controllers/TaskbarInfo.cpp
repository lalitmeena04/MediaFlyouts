#include "TaskbarInfo.h"

#include <QGuiApplication>
#include <QScreen>
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
