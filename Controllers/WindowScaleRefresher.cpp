#include "WindowScaleRefresher.h"

#include <QEvent>
#include <QGuiApplication>
#include <QPointer>
#include <QResizeEvent>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#ifdef Q_OS_WIN
#include <QtGui/qscreen_platform.h>
#include <windows.h>

#include <shellscalingapi.h>
#endif

#include "Logging.h"
#include "MonitorMapper.h"

namespace {

std::string nameOf(const QWindow* window) {
    const QString name = window->objectName();
    return (name.isEmpty() ? QString::fromLatin1(window->metaObject()->className()) : name).toStdString();
}

// DPI the native window renders at. Windows updates it on WM_DPICHANGED, which
// hidden windows get only when shown.
int nativeDpiOf(QWindow* window) {
#ifdef Q_OS_WIN
    if(window->handle())
        return static_cast<int>(GetDpiForWindow(reinterpret_cast<HWND>(window->winId())));
#endif
    return 0;
}

// DPI of the screen Qt binds the window to.
int screenDpiOf(const QWindow* window) {
    const QScreen* screen = window->screen();
    if(!screen)
        return 0;
#ifdef Q_OS_WIN
    if(auto* native = screen->nativeInterface<QNativeInterface::QWindowsScreen>()) {
        UINT x = 0;
        UINT y = 0;
        if(native->handle() && SUCCEEDED(GetDpiForMonitor(native->handle(), MDT_EFFECTIVE_DPI, &x, &y)))
            return static_cast<int>(x);
    }
#endif
    return qRound(screen->devicePixelRatio() * 96.0);
}

} // namespace

WindowScaleRefresher::WindowScaleRefresher(const MonitorMapper* monitors, QObject* parent) : QObject(parent) {
    if(monitors)
        connect(monitors, &MonitorMapper::changed, this, &WindowScaleRefresher::refreshAll);
    if(QCoreApplication* app = QCoreApplication::instance())
        app->installEventFilter(this);
}

bool WindowScaleRefresher::eventFilter(QObject* watched, QEvent* event) {
    const QEvent::Type type = event->type();
    if(type == QEvent::Resize || type == QEvent::Show || type == QEvent::DevicePixelRatioChange) {
        if(auto* window = qobject_cast<QWindow*>(watched); window && window->isTopLevel()) {
            if(type == QEvent::Resize) {
                Tracked& tracked = track(window);
                tracked.reportedSize = static_cast<QResizeEvent*>(event)->size();
                const mf::WindowScaleState state = stateOf(window, tracked);
                if(mf::resizeIsAuthoritative(tracked.snapshot, state)) {
                    tracked.snapshot = mf::WindowScaleSnapshot{state.currentSize, state.devicePixelRatio, state.nativeDpi};
                    tracked.attempts = 0;
                } else {
                    TRC_DEBUG("scale refresh: '{}' resize {}x{} not authoritative (current {}x{} dpr {} window dpi {} "
                              "screen dpi {}); checking later",
                              nameOf(window), state.reportedSize.width(), state.reportedSize.height(),
                              state.currentSize.width(), state.currentSize.height(), state.devicePixelRatio,
                              state.nativeDpi, state.screenDpi);
                    scheduleRefresh(window, 0);
                }
            } else {
                // Show: WM_DPICHANGED for a hidden window arrives inside the show.
                // DPR change: Qt moved the window to another screen without a resize.
                scheduleRefresh(window, 0);
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

WindowScaleRefresher::Tracked& WindowScaleRefresher::track(QWindow* window) {
    auto it = m_windows.find(window);
    if(it != m_windows.end())
        return *it;
    connect(window, &QObject::destroyed, this, [this, window] { m_windows.remove(window); });
    Tracked tracked;
    const mf::WindowScaleState state = stateOf(window, tracked);
    // Adopt the size only when it can be trusted. Else wait for a settled state.
    if(mf::scaleIsSettled(state))
        tracked.snapshot = mf::WindowScaleSnapshot{state.currentSize, state.devicePixelRatio, state.nativeDpi};
    return *m_windows.insert(window, tracked);
}

mf::WindowScaleState WindowScaleRefresher::stateOf(QWindow* window, const Tracked& tracked) const {
    mf::WindowScaleState state;
    state.reportedSize = tracked.reportedSize;
    state.currentSize = window->size();
    state.devicePixelRatio = window->devicePixelRatio();
    state.nativeDpi = nativeDpiOf(window);
    state.screenDpi = screenDpiOf(window);
    return state;
}

void WindowScaleRefresher::scheduleRefresh(QWindow* window, int delayMs) {
    QPointer<QWindow> guard(window);
    QTimer::singleShot(delayMs, this, [this, guard] {
        if(guard)
            refreshWindow(guard);
    });
}

void WindowScaleRefresher::refreshAll() {
    const QWindowList windows = QGuiApplication::allWindows();
    for(QWindow* window : windows) {
        if(window->isTopLevel())
            refreshWindow(window);
    }
}

void WindowScaleRefresher::refreshWindow(QWindow* window) {
    Tracked& tracked = track(window);
    const mf::WindowScaleState state = stateOf(window, tracked);

    if(!mf::scaleIsSettled(state)) {
        TRC_DEBUG("scale refresh: '{}' unsettled, window dpi {} screen dpi {} (current {}x{} dpr {}); waiting",
                  nameOf(window), state.nativeDpi, state.screenDpi, state.currentSize.width(),
                  state.currentSize.height(), state.devicePixelRatio);
        return;
    }

    const QSize restore = mf::sizeToRestore(tracked.snapshot, state);
    if(!restore.isValid()) {
        TRC_DEBUG("scale refresh: '{}' ok {}x{} dpr {} dpi {}", nameOf(window), state.currentSize.width(),
                  state.currentSize.height(), state.devicePixelRatio, state.nativeDpi);
        tracked.snapshot = mf::WindowScaleSnapshot{state.currentSize, state.devicePixelRatio, state.nativeDpi};
        tracked.attempts = 0;
        return;
    }

    if(tracked.attempts >= kMaxRestoreAttempts) {
        TRC_WARN("scale refresh: '{}' still {}x{} (reported {}x{}) after {} attempts; keeping it", nameOf(window),
                 state.currentSize.width(), state.currentSize.height(), state.reportedSize.width(),
                 state.reportedSize.height(), tracked.attempts);
        tracked.snapshot = mf::WindowScaleSnapshot{state.currentSize, state.devicePixelRatio, state.nativeDpi};
        tracked.attempts = 0;
        return;
    }

    ++tracked.attempts;
    TRC_INFO("scale refresh: '{}' restore {}x{} (current {}x{} reported {}x{} dpr {} -> {} dpi {} -> {}) attempt {}",
             nameOf(window), restore.width(), restore.height(), state.currentSize.width(),
             state.currentSize.height(), state.reportedSize.width(), state.reportedSize.height(),
             tracked.snapshot.devicePixelRatio, state.devicePixelRatio, tracked.snapshot.nativeDpi, state.nativeDpi,
             tracked.attempts);
    tracked.snapshot = mf::WindowScaleSnapshot{restore, state.devicePixelRatio, state.nativeDpi};
    // The resize events this causes carry the restored size and get recorded.
    if(mf::needsNudge(restore, state))
        window->resize(restore + QSize(1, 0));
    window->resize(restore);
    scheduleRefresh(window, kVerifyDelayMs);
}
