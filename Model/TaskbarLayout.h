#pragma once

#include <QRect>

namespace mf {

enum class TaskbarSide { Left = 0, Center = 1, Right = 2 };

// An empty rect means the shell did not report that element.
struct TaskbarReserve {
        QRect leading;  // Start and task buttons
        QRect trailing; // tray, clock and show-desktop button
};

inline constexpr int kUnmeasuredTrayWidth = 200;
inline constexpr int kEdgeTolerance = 8;

// Drops reserve rects that do not touch the screen. Shell chrome read for
// another monitor must not steer a widget on this one.
TaskbarReserve reserveOnScreen(const QRect& screen, TaskbarReserve reserve);

int taskbarWidgetX(const QRect& screen, const TaskbarReserve& reserve, TaskbarSide side, int offset, int width,
                   int margin);

} // namespace mf
