#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>

class QWindow;

// Locates the Windows taskbar and system-tray rectangles, in Qt logical pixels.
class TaskbarInfo : public QObject {
        Q_OBJECT

    public:
        explicit TaskbarInfo(QObject* parent = nullptr);

        // whole primary taskbar; empty QRect if it can't be found
        Q_INVOKABLE QRect taskbarGeometry() const;
        // notification area (tray icons + clock); empty QRect if it can't be found
        Q_INVOKABLE QRect trayGeometry() const;
        // top-left for the taskbar widget on the given monitor (side: 0 = left, 1 = right)
        Q_INVOKABLE QPoint widgetPosition(int monitor, int side, int offset, int width, int height) const;
        // full geometry (logical px) of the given monitor, used to keep flyouts on-screen
        Q_INVOKABLE QRect monitorGeometry(int monitor) const;
        // re-assert the window at the top of the topmost band so it stays above the taskbar
        Q_INVOKABLE void pinAboveTaskbar(QWindow* window) const;
};
