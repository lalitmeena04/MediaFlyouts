#pragma once

#include <QHash>
#include <QObject>

#include "WindowScalePolicy.h"

class QWindow;
class MonitorMapper;

// Qt skips the WM_DPICHANGED resize when a window is inside setGeometry(). The window
// keeps its old native size and QML size bindings do not re-run. This puts the size back.
class WindowScaleRefresher : public QObject {
        Q_OBJECT

    public:
        explicit WindowScaleRefresher(const MonitorMapper* monitors, QObject* parent = nullptr);

        bool eventFilter(QObject* watched, QEvent* event) override;

    public slots:
        void refreshAll();

    private:
        void refreshWindow(QWindow* window);
        void track(QWindow* window);
        void record(QWindow* window);

        QHash<QWindow*, mf::WindowScaleSnapshot> m_snapshots;
};
