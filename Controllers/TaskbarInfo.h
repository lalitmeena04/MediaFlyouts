#pragma once

#include <QList>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QRect>
#include <QTimer>

class QWindow;

// Finds the Windows taskbar and tray in Qt logical pixels.
class TaskbarInfo : public QObject {
        Q_OBJECT

    public:
        explicit TaskbarInfo(QObject* parent = nullptr);
        ~TaskbarInfo() override;

        // Primary taskbar. Empty if not found.
        Q_INVOKABLE QRect taskbarGeometry() const;
        // Tray area. Empty if not found.
        Q_INVOKABLE QRect trayGeometry() const;
        // Top-left for the taskbar widget.
        Q_INVOKABLE QPoint widgetPosition(int monitor, int side, int offset, int width, int height) const;
        // Full monitor geometry in logical pixels.
        Q_INVOKABLE QRect monitorGeometry(int monitor) const;
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
        mutable QList<QPointer<QWindow>> m_pinned;
        QTimer m_repinBurst;
        int m_repinBurstTicks = 0;
};


