#include "SwipeRecognizer.h"

#include <QtMath>

namespace mf {

SwipeDirection recognizeSwipe(QPointF delta, int elapsedMs, const SwipeThresholds& thresholds) {
    if(elapsedMs < 0 || elapsedMs > thresholds.maxDurationMs)
        return SwipeDirection::None;

    const qreal ax = qAbs(delta.x());
    const qreal ay = qAbs(delta.y());
    const bool horizontal = ax >= ay;
    const qreal major = horizontal ? ax : ay;
    const qreal minor = horizontal ? ay : ax;

    if(major < thresholds.minDistance)
        return SwipeDirection::None;
    if(minor > major * thresholds.maxCrossRatio)
        return SwipeDirection::None;

    if(horizontal)
        return delta.x() < 0 ? SwipeDirection::Left : SwipeDirection::Right;
    return delta.y() < 0 ? SwipeDirection::Up : SwipeDirection::Down;
}

SwipeAxis lockSwipeAxis(QPointF delta, SwipeAxis current, const SwipeThresholds& thresholds) {
    if(current != SwipeAxis::None)
        return current;
    const qreal ax = qAbs(delta.x());
    const qreal ay = qAbs(delta.y());
    if(qMax(ax, ay) < thresholds.axisLockDistance)
        return SwipeAxis::None;
    return ax >= ay ? SwipeAxis::Horizontal : SwipeAxis::Vertical;
}

QPointF projectOnAxis(QPointF delta, SwipeAxis axis) {
    switch(axis) {
    case SwipeAxis::Horizontal:
        return QPointF(delta.x(), 0);
    case SwipeAxis::Vertical:
        return QPointF(0, delta.y());
    case SwipeAxis::None:
        break;
    }
    return QPointF(0, 0);
}

QPointF swipeVector(SwipeDirection direction) {
    switch(direction) {
    case SwipeDirection::Left:
        return QPointF(-1, 0);
    case SwipeDirection::Right:
        return QPointF(1, 0);
    case SwipeDirection::Up:
        return QPointF(0, -1);
    case SwipeDirection::Down:
        return QPointF(0, 1);
    case SwipeDirection::None:
        break;
    }
    return QPointF(0, 0);
}

} // namespace mf
