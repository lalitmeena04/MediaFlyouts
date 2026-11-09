#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QRect>
#include <QTimer>

#include <optional>

#include "TaskbarLayout.h"

class QWindow;
class MonitorMapper;
struct MonitorEntry;

class TaskbarInfo : public QObject {
        Q_OBJECT

    public:
        explicit TaskbarInfo(const MonitorMapper* monitors, QObject* parent = nullptr);
        ~TaskbarInfo() override;

        Q_INVOKABLE QPoint widgetPosition(int monitor, int side, int offset, int width, int height) const;
        // mf::TaskbarEdge as int. Bottom when the monitor is unknown.
        Q_INVOKABLE int taskbarEdge(int monitor) const;
        // The widgets are horizontal strips; a left or right taskbar cannot hold them.
        Q_INVOKABLE bool widgetFits(int monitor) const;
        Q_INVOKABLE void pinTopmost(QWindow* window) const;
        Q_INVOKABLE void pinAboveTaskbar(QWindow* window) const;

        void repinTracked() const;
        void handleShellEvent(unsigned long event);

    private:
        mf::TaskbarReserve reserveFor(const MonitorEntry& monitor) const;
        mf::TaskbarEdge edgeFor(const MonitorEntry& monitor) const;

        // The automation walk is slow. Widgets poll every 500 ms. Cache briefly.
        struct CachedReserve {
                mf::TaskbarReserve reserve;
                QElapsedTimer age;
                QRect availableRect;
                qreal devicePixelRatio = 1.0;
        };
        bool cacheIsFresh(const CachedReserve& entry, const MonitorEntry& monitor) const;
        mutable QHash<void*, CachedReserve> m_reserveCache;
        // Last edge per monitor, so the log records moves only.
        mutable QHash<int, std::optional<mf::TaskbarEdge>> m_lastEdge;

        const MonitorMapper* m_monitors = nullptr;
        mutable QList<QPointer<QWindow>> m_pinned;
        QTimer m_repinBurst;
        int m_repinBurstTicks = 0;
};


