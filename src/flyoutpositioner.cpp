#include "flyoutpositioner.h"

namespace mf {

QRect computeFlyoutRect(const QRect& workArea, const QSize& flyoutSize, FlyoutPosition position, int margin)
{
    const int w = flyoutSize.width();
    const int h = flyoutSize.height();
    int x = workArea.left();
    int y = workArea.top();

    switch (position) {
    case FlyoutPosition::TopLeft:
    case FlyoutPosition::BottomLeft:
        x = workArea.left() + margin;
        break;
    case FlyoutPosition::TopCenter:
    case FlyoutPosition::BottomCenter:
        x = workArea.left() + (workArea.width() - w) / 2;
        break;
    case FlyoutPosition::TopRight:
    case FlyoutPosition::BottomRight:
        x = workArea.right() + 1 - w - margin;
        break;
    }

    switch (position) {
    case FlyoutPosition::TopLeft:
    case FlyoutPosition::TopCenter:
    case FlyoutPosition::TopRight:
        y = workArea.top() + margin;
        break;
    case FlyoutPosition::BottomLeft:
    case FlyoutPosition::BottomCenter:
    case FlyoutPosition::BottomRight:
        y = workArea.bottom() + 1 - h - margin;
        break;
    }

    return QRect(QPoint(x, y), flyoutSize);
}

} // namespace mf
