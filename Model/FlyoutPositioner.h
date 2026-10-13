#pragma once

#include <QRect>
#include <QSize>

namespace mf {

enum class FlyoutPosition { TopLeft, TopCenter, TopRight, BottomLeft, BottomCenter, BottomRight };

QRect computeFlyoutRect(const QRect& workArea, const QSize& flyoutSize, FlyoutPosition position, int margin);

} // namespace mf
