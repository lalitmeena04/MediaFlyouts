#pragma once

#include <QPointF>
#include <QtGlobal>

namespace mf {

// Order matches the int values QML sees through SwipeDetector.
enum class SwipeDirection { None = 0, Left = 1, Right = 2, Up = 3, Down = 4 };

enum class SwipeAxis { None = 0, Horizontal = 1, Vertical = 2 };

struct SwipeThresholds {
        qreal minDistance = 48;
        // Travel on the other axis divided by the main axis. Larger is a diagonal, not a swipe.
        qreal maxCrossRatio = 0.6;
        int maxDurationMs = 600;
        qreal axisLockDistance = 8;
};

SwipeDirection recognizeSwipe(QPointF delta, int elapsedMs, const SwipeThresholds& thresholds = {});

SwipeAxis lockSwipeAxis(QPointF delta, SwipeAxis current, const SwipeThresholds& thresholds = {});

QPointF projectOnAxis(QPointF delta, SwipeAxis axis);

QPointF swipeVector(SwipeDirection direction);

} // namespace mf
