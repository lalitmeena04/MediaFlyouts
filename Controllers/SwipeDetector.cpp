#include "SwipeDetector.h"

#include "Logging.h"

SwipeDetector::SwipeDetector(QObject* parent) : QObject(parent) {}

int SwipeDetector::recognize(qreal dx, qreal dy, int elapsedMs) const {
    const auto direction = mf::recognizeSwipe(QPointF(dx, dy), elapsedMs, m_thresholds);
    TRC_DEBUG("swipe: dx={} dy={} ms={} -> {}", dx, dy, elapsedMs, static_cast<int>(direction));
    return static_cast<int>(direction);
}

int SwipeDetector::lockAxis(qreal dx, qreal dy, int currentAxis) const {
    return static_cast<int>(mf::lockSwipeAxis(QPointF(dx, dy), static_cast<mf::SwipeAxis>(currentAxis), m_thresholds));
}

QPointF SwipeDetector::project(qreal dx, qreal dy, int axis) const {
    return mf::projectOnAxis(QPointF(dx, dy), static_cast<mf::SwipeAxis>(axis));
}

QPointF SwipeDetector::vector(int direction) const {
    return mf::swipeVector(static_cast<mf::SwipeDirection>(direction));
}

qreal SwipeDetector::minDistance() const {
    return m_thresholds.minDistance;
}
