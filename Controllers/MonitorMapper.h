#pragma once

#include <QList>
#include <QObject>
#include <QRect>
#include <QStringList>
#include <QTimer>

class QScreen;

struct MonitorEntry {
        int index = -1;
        QScreen* screen = nullptr;
        void* nativeHandle = nullptr;
        QRect logicalRect;
        QRect availableRect;
        QRect nativeRect;
        qreal devicePixelRatio = 1.0;
        bool primary = false;
};

class MonitorMapper : public QObject {
        Q_OBJECT
        Q_PROPERTY(int count READ count NOTIFY changed)
        Q_PROPERTY(QStringList names READ names NOTIFY changed)

    public:
        // Display changes arrive in bursts. One changed() per burst.
        static constexpr int kDefaultCoalesceMs = 50;

        explicit MonitorMapper(QObject* parent = nullptr, int coalesceMs = kDefaultCoalesceMs);

        int count() const;
        QStringList names() const;
        int clampIndex(int index) const;
        const MonitorEntry* entry(int index) const;

        Q_INVOKABLE QRect geometry(int index) const;
        Q_INVOKABLE QRect availableGeometry(int index) const;

        Q_INVOKABLE void refresh();

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
