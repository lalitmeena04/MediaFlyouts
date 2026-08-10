#pragma once

#include <QRect>
#include <QSize>

namespace mf {

enum class FlyoutPosition { TopLeft, TopCenter, TopRight, BottomLeft, BottomCenter, BottomRight };

// Computes the placement rect for a flyout of flyoutSize inside workArea, honoring an edge margin.
QRect computeFlyoutRect(const QRect& workArea, const QSize& flyoutSize, FlyoutPosition position, int margin);

} // namespace mf
