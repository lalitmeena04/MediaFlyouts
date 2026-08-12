#pragma once

#include <QObject>
#include <QRect>

// Locates the Windows taskbar and system-tray rectangles, in Qt logical pixels.
class TaskbarInfo : public QObject {
        Q_OBJECT

    public:
        explicit TaskbarInfo(QObject* parent = nullptr);

        // whole primary taskbar; empty QRect if it can't be found
        Q_INVOKABLE QRect taskbarGeometry() const;
        // notification area (tray icons + clock); empty QRect if it can't be found
        Q_INVOKABLE QRect trayGeometry() const;
};
