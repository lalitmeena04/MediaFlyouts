#pragma once

#include <QList>
#include <QObject>
#include <QRect>
#include <QStringList>
#include <QTimer>

class QScreen;

// One connected display. Index follows QGuiApplication::screens().
// That is the order the monitor combo boxes show and settings store.
struct MonitorEntry {
        int index = -1;
        QScreen* screen = nullptr;
        // HMONITOR on Windows. Null when the platform has none.
        void* nativeHandle = nullptr;
        // Qt logical pixels. Same as QScreen::geometry().
        QRect logicalRect;
        // Logical pixels minus the taskbar. Same as QScreen::availableGeometry().
        QRect availableRect;
        // Physical pixels in virtual-desktop coordinates.
        QRect nativeRect;
        qreal devicePixelRatio = 1.0;
        bool primary = false;
};

// Single source of monitor facts for the app.
// Controllers resolve a saved monitor index here instead of walking
// QGuiApplication::screens() and mapping to HMONITOR on their own.
class MonitorMapper : public QObject {
        Q_OBJECT
        Q_PROPERTY(int count READ count NOTIFY changed)
        Q_PROPERTY(QStringList names READ names NOTIFY changed)

    public:
        // Display changes arrive in bursts. One changed() per burst.
        static constexpr int kDefaultCoalesceMs = 50;

        explicit MonitorMapper(QObject* parent = nullptr, int coalesceMs = kDefaultCoalesceMs);

        int count() const;
        // "1. Name (WxH)" per monitor, W and H in native pixels. Feeds the
        // settings combo boxes. Refreshes with changed() when monitors come,
        // go, or change mode.
        QStringList names() const;
        // A saved index can outlive its monitor. Out of range falls back to 0.
        int clampIndex(int index) const;
        // Null only when no screen exists, such as a guiless test host.
        const MonitorEntry* entry(int index) const;

        Q_INVOKABLE QRect geometry(int index) const;
        Q_INVOKABLE QRect availableGeometry(int index) const;

        // Drop the cache and emit changed() after the coalesce window.
        // Screen signals call this. Tests and callers may call it too.
        Q_INVOKABLE void refresh();

        // Convert a native rect that lies on this monitor to logical pixels.
        // Uses that monitor's own scale. The primary scale is wrong when
        // monitors differ.
        static QRect toLogical(const MonitorEntry& monitor, const QRect& nativeRect);

    signals:
        void changed();

    private:
        void watchScreen(QScreen* screen);
        void rebuildIfDirty() const;

        mutable QList<MonitorEntry> m_entries;
        mutable bool m_dirty = true;
        QTimer m_coalesce;
};
