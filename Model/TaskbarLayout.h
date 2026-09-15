#pragma once

#include <QRect>

namespace mf {

enum class TaskbarSide { Left = 0, Center = 1, Right = 2 };

// Taskbar chrome a widget must not cover, in screen logical pixels.
// An empty rect means the shell did not report that element.
struct TaskbarReserve {
        QRect leading;  // Start and task buttons
        QRect trailing; // tray, clock and show-desktop button
};

// Width held back on the right when the tray cannot be measured.
inline constexpr int kUnmeasuredTrayWidth = 200;
// A reserve counts only when it sits this close to its screen edge.
inline constexpr int kEdgeTolerance = 8;

// Left edge for a taskbar widget of the given width on one screen.
int taskbarWidgetX(const QRect& screen, const TaskbarReserve& reserve, TaskbarSide side, int offset, int width,
                   int margin);

} // namespace mf
