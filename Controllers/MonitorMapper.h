#pragma once

#include <QList>
#include <QObject>
#include <QRect>
#include <QStringList>

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
        explicit MonitorMapper(QObject* parent = nullptr);

        int count() const;
        // "1. Name (WxH)" per monitor. Feeds the settings combo boxes.
        QStringList names() const;
        // A saved index can outlive its monitor. Out of range falls back to 0.
        int clampIndex(int index) const;
        // Null only when no screen exists, such as a guiless test host.
        const MonitorEntry* entry(int index) const;

        Q_INVOKABLE QRect geometry(int index) const;
        Q_INVOKABLE QRect availableGeometry(int index) const;

        // Convert a native rect that lies on this monitor to logical pixels.
        // Uses that monitor's own scale. The primary scale is wrong when
        // monitors differ.
        static QRect toLogical(const MonitorEntry& monitor, const QRect& nativeRect);

    signals:
        void changed();

    private:
        void watchScreen(QScreen* screen);
        void invalidate();
        void rebuildIfDirty() const;

        mutable QList<MonitorEntry> m_entries;
        mutable bool m_dirty = true;
};
