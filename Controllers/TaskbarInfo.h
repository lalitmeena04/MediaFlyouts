#pragma once

#include <QList>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QRect>
#include <QTimer>

class QWindow;

// Locates the Windows taskbar and system-tray rectangles, in Qt logical pixels.
class TaskbarInfo : public QObject {
        Q_OBJECT

    public:
        explicit TaskbarInfo(QObject* parent = nullptr);
        ~TaskbarInfo() override;

        // whole primary taskbar; empty QRect if it can't be found
        Q_INVOKABLE QRect taskbarGeometry() const;
        // notification area (tray icons + clock); empty QRect if it can't be found
        Q_INVOKABLE QRect trayGeometry() const;
        // top-left for the taskbar widget on the given monitor (side: 0 = left, 1 = right)
        Q_INVOKABLE QPoint widgetPosition(int monitor, int side, int offset, int width, int height) const;
        // full geometry (logical px) of the given monitor, used to keep flyouts on-screen
        Q_INVOKABLE QRect monitorGeometry(int monitor) const;
        // re-assert the window at the top of the topmost band. The window is remembered and
        // re-pinned automatically on foreground/minimize changes.
        Q_INVOKABLE void pinTopmost(QWindow* window) const;
        // compatibility name for taskbar widgets docked over the shell taskbar
        Q_INVOKABLE void pinAboveTaskbar(QWindow* window) const;

        // re-pin every tracked window right now; called from the WinEvent hook
        void repinTracked() const;
        // react to a shell WinEvent: re-pin now and, for minimize/foreground changes, keep re-pinning
        // for a short burst to cover the taskbar re-topping itself across the minimize animation
        void handleShellEvent(unsigned long event);

    private:
        mutable QList<QPointer<QWindow>> m_pinned;
        QTimer m_repinBurst;
        int m_repinBurstTicks = 0;
};


