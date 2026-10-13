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
