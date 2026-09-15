#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QRect>
#include <QTimer>

#include "TaskbarLayout.h"

class QWindow;
class MonitorMapper;
struct MonitorEntry;

// Finds the Windows taskbar and tray in Qt logical pixels.
class TaskbarInfo : public QObject {
        Q_OBJECT

    public:
        explicit TaskbarInfo(const MonitorMapper* monitors, QObject* parent = nullptr);
        ~TaskbarInfo() override;

        // Top-left for the taskbar widget.
        Q_INVOKABLE QPoint widgetPosition(int monitor, int side, int offset, int width, int height) const;
        // Put the window above other topmost windows.
        // Remember it for later shell events.
        Q_INVOKABLE void pinTopmost(QWindow* window) const;
        // Old name used by taskbar widgets.
        Q_INVOKABLE void pinAboveTaskbar(QWindow* window) const;

        // Pin all tracked windows now.
        void repinTracked() const;
        // Handle one shell event.
        // Pin more during minimize and foreground changes.
        void handleShellEvent(unsigned long event);

    private:
        // Chrome to avoid on the taskbar that belongs to this monitor.
        mf::TaskbarReserve reserveFor(const MonitorEntry& monitor) const;

        // The automation walk is slow. Widgets poll every 500 ms. Cache briefly.
        struct CachedReserve {
                mf::TaskbarReserve reserve;
                QElapsedTimer age;
        };
        mutable QHash<void*, CachedReserve> m_reserveCache;

        const MonitorMapper* m_monitors = nullptr;
        mutable QList<QPointer<QWindow>> m_pinned;
        QTimer m_repinBurst;
        int m_repinBurstTicks = 0;
};


