#pragma once

#include <QPointF>
#include <QtGlobal>

namespace mf {

// Order matches the int values QML sees through SwipeDetector.
enum class SwipeDirection { None = 0, Left = 1, Right = 2, Up = 3, Down = 4 };

// Axis a drag is locked to. The content moves in a straight line along it.
enum class SwipeAxis { None = 0, Horizontal = 1, Vertical = 2 };

struct SwipeThresholds {
        // Travel along the main axis, in logical px.
        qreal minDistance = 48;
        // Travel on the other axis divided by the main axis. Larger is a diagonal, not a swipe.
        qreal maxCrossRatio = 0.6;
        // Slower than this is a drag the user may still cancel, not a swipe.
        int maxDurationMs = 600;
        // Travel before the drag axis is chosen. Avoids flicker near 45 degrees.
        qreal axisLockDistance = 8;
};

// Pure. The larger axis decides the direction. None when a threshold fails.
SwipeDirection recognizeSwipe(QPointF delta, int elapsedMs, const SwipeThresholds& thresholds = {});

// Keeps a locked axis. Otherwise picks the larger axis once travel reaches the lock distance.
SwipeAxis lockSwipeAxis(QPointF delta, SwipeAxis current, const SwipeThresholds& thresholds = {});

// Drops the component off the axis. None gives (0, 0).
QPointF projectOnAxis(QPointF delta, SwipeAxis axis);

// Unit vector that points the way the content leaves the screen. None gives (0, 0).
QPointF swipeVector(SwipeDirection direction);

} // namespace mf
