#pragma once

#include <QHash>
#include <QObject>
#include <QSize>

#include "WindowScalePolicy.h"

class QWindow;
class MonitorMapper;

// Keeps the logical size of every top-level window equal to the size the app
// asked for across DPI changes.
//
// Windows defers WM_DPICHANGED for hidden windows. After a monitor is removed or
// added, a hidden flyout can sit on a screen whose DPI differs from the DPI its
// native window still has. Qt then scales the native size for the new screen,
// and Windows scales it again when the window is shown. The result is a window
// that is 1.5x or 2x too large (or too small) with content at the normal size.
//
// This class treats a window as "settled" only when its native DPI equals the
// DPI of the screen Qt binds it to. Sizes reported while unsettled are ignored.
// Once settled, any drift from the last size the app asked for is undone.
class WindowScaleRefresher : public QObject {
        Q_OBJECT

    public:
        static constexpr int kMaxRestoreAttempts = 3;
        // Qt delivers platform geometry events after posted events. A later check
        // sees the final state instead of the half-applied one.
        static constexpr int kVerifyDelayMs = 50;

        explicit WindowScaleRefresher(const MonitorMapper* monitors, QObject* parent = nullptr);

        bool eventFilter(QObject* watched, QEvent* event) override;

    public slots:
        void refreshAll();

    private:
        struct Tracked {
                mf::WindowScaleSnapshot snapshot;
                QSize reportedSize;
                int attempts = 0;
        };

        void refreshWindow(QWindow* window);
        void scheduleRefresh(QWindow* window, int delayMs);
        Tracked& track(QWindow* window);
        mf::WindowScaleState stateOf(QWindow* window, const Tracked& tracked) const;

        QHash<QWindow*, Tracked> m_windows;
};
