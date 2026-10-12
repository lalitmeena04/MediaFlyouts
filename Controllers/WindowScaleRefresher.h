#pragma once

#include <QHash>
#include <QObject>

#include "WindowScalePolicy.h"

class QWindow;
class MonitorMapper;

// Keeps the logical size of every top-level window stable across a
// scale change. Qt skips the WM_DPICHANGED resize when a window is inside
// setGeometry() at that moment. The window keeps its old native size and
// Qt reports a smaller logical size. QML size bindings do not re-run, so
// the window stays small. This class puts the size back.
class WindowScaleRefresher : public QObject {
        Q_OBJECT

    public:
        explicit WindowScaleRefresher(const MonitorMapper* monitors, QObject* parent = nullptr);

        bool eventFilter(QObject* watched, QEvent* event) override;

    public slots:
        // Check every top-level window after a monitor change.
        void refreshAll();

    private:
        // Resize one window if its native size did not follow the scale.
        void refreshWindow(QWindow* window);
        void track(QWindow* window);
        void record(QWindow* window);

        QHash<QWindow*, mf::WindowScaleSnapshot> m_snapshots;
};
