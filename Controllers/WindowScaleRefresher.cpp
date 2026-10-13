#include "WindowScaleRefresher.h"

#include <QEvent>
#include <QGuiApplication>
#include <QPointer>
#include <QTimer>
#include <QWindow>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "Logging.h"
#include "MonitorMapper.h"

namespace {

// Size of the native window. Qt's logical size can lag behind it.
QSize nativeSize(QWindow* window) {
#ifdef Q_OS_WIN
    if(window->handle()) {
        RECT r{};
        if(GetWindowRect(reinterpret_cast<HWND>(window->winId()), &r))
            return QSize(r.right - r.left, r.bottom - r.top);
    }
#endif
    return mf::nativeSizeOf(window->size(), window->devicePixelRatio());
}

} // namespace

WindowScaleRefresher::WindowScaleRefresher(const MonitorMapper* monitors, QObject* parent) : QObject(parent) {
    if(monitors)
        connect(monitors, &MonitorMapper::changed, this, &WindowScaleRefresher::refreshAll);
    if(QCoreApplication* app = QCoreApplication::instance())
        app->installEventFilter(this);
}

bool WindowScaleRefresher::eventFilter(QObject* watched, QEvent* event) {
    if(event->type() == QEvent::Resize) {
        if(auto* window = qobject_cast<QWindow*>(watched); window && window->isTopLevel()) {
            auto it = m_snapshots.find(window);
            if(it == m_snapshots.end()) {
                track(window);
            } else if(mf::resizeIsAuthoritative(*it, window->devicePixelRatio())) {
                record(window);
            } else {
                QPointer<QWindow> guard(window);
                QTimer::singleShot(0, this, [this, guard] {
                    if(guard)
                        refreshWindow(guard);
                });
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void WindowScaleRefresher::track(QWindow* window) {
    connect(window, &QObject::destroyed, this, [this, window] { m_snapshots.remove(window); });
    record(window);
}

void WindowScaleRefresher::record(QWindow* window) {
    m_snapshots[window] = mf::WindowScaleSnapshot{window->size(), window->devicePixelRatio()};
}

void WindowScaleRefresher::refreshAll() {
    const QWindowList windows = QGuiApplication::allWindows();
    for(QWindow* window : windows) {
        if(window->isTopLevel())
            refreshWindow(window);
    }
}

void WindowScaleRefresher::refreshWindow(QWindow* window) {
    auto it = m_snapshots.find(window);
    if(it == m_snapshots.end()) {
        track(window);
        return;
    }
    const qreal dpr = window->devicePixelRatio();
    const QSize native = nativeSize(window);
    const QSize restore = mf::sizeToRestore(*it, native, dpr);
    if(restore.isValid()) {
        TRC_INFO("scale refresh: '{}' native {}x{} logical {}x{} -> {}x{} dpr {} -> {}",
                 window->objectName().toStdString(), native.width(), native.height(), window->width(),
                 window->height(), restore.width(), restore.height(), it->devicePixelRatio, dpr);
        window->resize(restore);
    } else {
        TRC_DEBUG("scale refresh: '{}' ok native {}x{} logical {}x{} dpr {}", window->objectName().toStdString(),
                  native.width(), native.height(), window->width(), window->height(), dpr);
    }
    // The resize above is not authoritative by the dpr rule. Record by hand.
    *it = mf::WindowScaleSnapshot{restore.isValid() ? restore : window->size(), dpr};
}
