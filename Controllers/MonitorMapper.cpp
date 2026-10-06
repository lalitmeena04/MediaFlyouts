#include "MonitorMapper.h"

#include <cstdint>

#include <QGuiApplication>
#include <QScreen>

#ifdef Q_OS_WIN
#include <QtGui/qscreen_platform.h>
#include <windows.h>
#endif

#include "Logging.h"

namespace {

// Ask Qt for the native monitor. Skip name or size matching, which
// breaks when QScreen::name() is the model string or monitors are twins.
void fillNative(MonitorEntry& entry) {
#ifdef Q_OS_WIN
    auto* native = entry.screen->nativeInterface<QNativeInterface::QWindowsScreen>();
    HMONITOR handle = native ? native->handle() : nullptr;
    entry.nativeHandle = handle;

    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if(handle && GetMonitorInfoW(handle, &info)) {
        const RECT& r = info.rcMonitor;
        entry.nativeRect = QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
        return;
    }
#endif
    // Qt keeps a screen origin equal in native and logical space.
    entry.nativeRect = QRect(entry.logicalRect.topLeft(), entry.logicalRect.size() * entry.devicePixelRatio);
}

} // namespace

MonitorMapper::MonitorMapper(QObject* parent) : QObject(parent) {
    // qGuiApp is null in a QCoreApplication test host. The map stays empty there.
    if(QGuiApplication* app = qGuiApp) {
        connect(app, &QGuiApplication::screenAdded, this, [this](QScreen* screen) {
            watchScreen(screen);
            invalidate();
        });
        connect(app, &QGuiApplication::screenRemoved, this, &MonitorMapper::invalidate);
        connect(app, &QGuiApplication::primaryScreenChanged, this, &MonitorMapper::invalidate);
        for(QScreen* screen : QGuiApplication::screens())
            watchScreen(screen);
    }
}

void MonitorMapper::watchScreen(QScreen* screen) {
    connect(screen, &QScreen::geometryChanged, this, &MonitorMapper::invalidate);
    connect(screen, &QScreen::availableGeometryChanged, this, &MonitorMapper::invalidate);
    connect(screen, &QScreen::logicalDotsPerInchChanged, this, &MonitorMapper::invalidate);
}

void MonitorMapper::invalidate() {
    m_dirty = true;
    emit changed();
}

void MonitorMapper::rebuildIfDirty() const {
    if(!m_dirty)
        return;
    m_dirty = false;
    m_entries.clear();

    const QList<QScreen*> screens = QGuiApplication::screens();
    const QScreen* primary = QGuiApplication::primaryScreen();
    for(int i = 0; i < screens.size(); ++i) {
        MonitorEntry entry;
        entry.index = i;
        entry.screen = screens.at(i);
        entry.logicalRect = entry.screen->geometry();
        entry.availableRect = entry.screen->availableGeometry();
        entry.devicePixelRatio = entry.screen->devicePixelRatio() > 0 ? entry.screen->devicePixelRatio() : 1.0;
        entry.primary = entry.screen == primary;
        fillNative(entry);
        m_entries.append(entry);

        TRC_DEBUG("monitor {} '{}' logical=({},{} {}x{}) available=({},{} {}x{}) native=({},{} {}x{}) dpr={} "
                  "handle={:#x} primary={}",
                  i, entry.screen->name().toStdString(), entry.logicalRect.x(), entry.logicalRect.y(),
                  entry.logicalRect.width(), entry.logicalRect.height(), entry.availableRect.x(),
                  entry.availableRect.y(), entry.availableRect.width(), entry.availableRect.height(),
                  entry.nativeRect.x(), entry.nativeRect.y(), entry.nativeRect.width(), entry.nativeRect.height(),
                  entry.devicePixelRatio, reinterpret_cast<std::uintptr_t>(entry.nativeHandle), entry.primary);
    }
}

int MonitorMapper::count() const {
    rebuildIfDirty();
    return m_entries.size();
}

QStringList MonitorMapper::names() const {
    rebuildIfDirty();
    QStringList names;
    for(const MonitorEntry& entry : m_entries) {
        names << QStringLiteral("%1. %2 (%3x%4)")
                     .arg(entry.index + 1)
                     .arg(entry.screen->name())
                     .arg(entry.logicalRect.width())
                     .arg(entry.logicalRect.height());
    }
    return names;
}

int MonitorMapper::clampIndex(int index) const {
    rebuildIfDirty();
    if(index < 0 || index >= m_entries.size())
        return 0;
    return index;
}

const MonitorEntry* MonitorMapper::entry(int index) const {
    rebuildIfDirty();
    if(m_entries.isEmpty())
        return nullptr;
    return &m_entries.at(clampIndex(index));
}

QRect MonitorMapper::geometry(int index) const {
    const MonitorEntry* e = entry(index);
    return e ? e->logicalRect : QRect();
}

QRect MonitorMapper::availableGeometry(int index) const {
    const MonitorEntry* e = entry(index);
    return e ? e->availableRect : QRect();
}

QRect MonitorMapper::toLogical(const MonitorEntry& monitor, const QRect& nativeRect) {
    if(nativeRect.isNull())
        return QRect();
    const qreal dpr = monitor.devicePixelRatio > 0 ? monitor.devicePixelRatio : 1.0;
    return QRect(monitor.logicalRect.left() + qRound((nativeRect.left() - monitor.nativeRect.left()) / dpr),
                 monitor.logicalRect.top() + qRound((nativeRect.top() - monitor.nativeRect.top()) / dpr),
                 qRound(nativeRect.width() / dpr), qRound(nativeRect.height() / dpr));
}
