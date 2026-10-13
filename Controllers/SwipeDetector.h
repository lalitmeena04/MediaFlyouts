#pragma once

#include <QObject>
#include <QPointF>

#include "SwipeRecognizer.h"

// Exposes mf::recognizeSwipe to QML as the context property "swipe".
// Stateless. QML keeps the press position and time.
class SwipeDetector : public QObject {
        Q_OBJECT

    public:
        explicit SwipeDetector(QObject* parent = nullptr);

        // 0 none, 1 left, 2 right, 3 up, 4 down. Same as mf::SwipeDirection.
        Q_INVOKABLE int recognize(qreal dx, qreal dy, int elapsedMs) const;
        // 0 none, 1 horizontal, 2 vertical. Same as mf::SwipeAxis. Keeps a locked axis.
        Q_INVOKABLE int lockAxis(qreal dx, qreal dy, int currentAxis) const;
        // Delta with the off-axis part removed.
        Q_INVOKABLE QPointF project(qreal dx, qreal dy, int axis) const;
        // Unit vector for the exit slide of a direction from recognize().
        Q_INVOKABLE QPointF vector(int direction) const;
        // Main-axis travel a gesture needs before it counts. For QML drag feedback.
        Q_INVOKABLE qreal minDistance() const;

    private:
        mf::SwipeThresholds m_thresholds;
};
