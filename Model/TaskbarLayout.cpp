#include "TaskbarLayout.h"

#include <QtGlobal>

namespace mf {
namespace {

// Chrome counts only when it touches the screen edge it belongs to.
// A centered Windows 11 task list must not push an edge-aligned widget inward.
bool holdsLeftEdge(const QRect& screen, const QRect& reserve) {
    return !reserve.isEmpty() && reserve.left() <= screen.left() + kEdgeTolerance && reserve.right() < screen.right();
}

bool holdsRightEdge(const QRect& screen, const QRect& reserve) {
    return !reserve.isEmpty() && reserve.right() >= screen.right() - kEdgeTolerance && reserve.left() > screen.left();
}

} // namespace

TaskbarReserve reserveOnScreen(const QRect& screen, TaskbarReserve reserve) {
    if(!reserve.leading.intersects(screen))
        reserve.leading = QRect();
    if(!reserve.trailing.intersects(screen))
        reserve.trailing = QRect();
    return reserve;
}

TaskbarEdge detectTaskbarEdge(const QRect& screen, const QRect& available, const QRect& taskbarWindow) {
    struct Gap {
            TaskbarEdge edge;
            int size;
    };
    const Gap gaps[] = {
        {TaskbarEdge::Bottom, screen.bottom() - available.bottom()},
        {TaskbarEdge::Top, available.top() - screen.top()},
        {TaskbarEdge::Left, available.left() - screen.left()},
        {TaskbarEdge::Right, screen.right() - available.right()},
    };
    const Gap* best = &gaps[0];
    for(const Gap& g : gaps)
        if(g.size > best->size)
            best = &g;
    if(best->size > 0)
        return best->edge;

    // Auto-hide: the window still sits on its edge, partly off screen.
    if(taskbarWindow.isEmpty() || !taskbarWindow.intersects(screen.adjusted(-1, -1, 1, 1)))
        return TaskbarEdge::Bottom;
    const QPoint c = taskbarWindow.center();
    const QPoint s = screen.center();
    if(taskbarWindow.width() >= taskbarWindow.height())
        return c.y() < s.y() ? TaskbarEdge::Top : TaskbarEdge::Bottom;
    return c.x() < s.x() ? TaskbarEdge::Left : TaskbarEdge::Right;
}

int taskbarWidgetY(const QRect& screen, const QRect& available, TaskbarEdge edge, int height) {
    int depth;
    int top;
    if(edge == TaskbarEdge::Top) {
        depth = available.top() - screen.top();
        if(depth <= 0)
            depth = kDefaultTaskbarDepth;
        top = screen.top();
    } else {
        depth = screen.bottom() - available.bottom();
        if(depth <= 0)
            depth = kDefaultTaskbarDepth;
        top = screen.bottom() + 1 - depth;
    }
    return top + qMax(0, (depth - height) / 2);
}

int taskbarWidgetX(const QRect& screen, const TaskbarReserve& reserve, TaskbarSide side, int offset, int width,
                   int margin) {
    int x = screen.left();
    if(side == TaskbarSide::Left) {
        int leftEdge = screen.left();
        if(holdsLeftEdge(screen, reserve.leading))
            leftEdge = reserve.leading.right() + 1;
        x = leftEdge + margin + qMax(0, offset);
    } else if(side == TaskbarSide::Center) {
        x = screen.left() + (screen.width() - width) / 2 + offset;
    } else {
        int rightEdge = screen.right() + 1 - kUnmeasuredTrayWidth;
        if(holdsRightEdge(screen, reserve.trailing))
            rightEdge = reserve.trailing.left();
        x = rightEdge - width - margin - qMax(0, offset);
    }

    const int maxX = qMax(screen.left(), screen.right() + 1 - width);
    return qBound(screen.left(), x, maxX);
}

} // namespace mf
