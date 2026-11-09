#pragma once

#include <QRect>

namespace mf {

enum class TaskbarSide { Left = 0, Center = 1, Right = 2 };

// Screen edge the taskbar is docked to. Numeric values are the QML contract.
enum class TaskbarEdge { Bottom = 0, Top = 1, Left = 2, Right = 3 };

// An empty rect means the shell did not report that element.
struct TaskbarReserve {
        QRect leading;  // Start and task buttons
        QRect trailing; // tray, clock and show-desktop button
};

inline constexpr int kUnmeasuredTrayWidth = 200;
inline constexpr int kEdgeTolerance = 8;
// Taskbar depth assumed when the work area reports no gap (auto-hide).
inline constexpr int kDefaultTaskbarDepth = 40;

// The edge with the largest work-area gap wins. With no gap (auto-hide) the
// taskbar window's own rect decides; with no window either, Bottom.
TaskbarEdge detectTaskbarEdge(const QRect& screen, const QRect& available, const QRect& taskbarWindow);

inline bool isHorizontalEdge(TaskbarEdge edge) {
    return edge == TaskbarEdge::Bottom || edge == TaskbarEdge::Top;
}

// Vertical centre of a widget of `height` inside the taskbar strip on a
// horizontal edge. Vertical edges fall back to the bottom strip; the widget
// is hidden there anyway.
int taskbarWidgetY(const QRect& screen, const QRect& available, TaskbarEdge edge, int height);

// Drops reserve rects that do not touch the screen. Shell chrome read for
// another monitor must not steer a widget on this one.
TaskbarReserve reserveOnScreen(const QRect& screen, TaskbarReserve reserve);

int taskbarWidgetX(const QRect& screen, const TaskbarReserve& reserve, TaskbarSide side, int offset, int width,
                   int margin);

} // namespace mf
