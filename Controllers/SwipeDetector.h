#pragma once

#include <QObject>
#include <QPointF>

#include "SwipeRecognizer.h"

class SwipeDetector : public QObject {
        Q_OBJECT

    public:
        explicit SwipeDetector(QObject* parent = nullptr);

        // 0 none, 1 left, 2 right, 3 up, 4 down. Same as mf::SwipeDirection.
        Q_INVOKABLE int recognize(qreal dx, qreal dy, int elapsedMs) const;
        // 0 none, 1 horizontal, 2 vertical. Same as mf::SwipeAxis. Keeps a locked axis.
        Q_INVOKABLE int lockAxis(qreal dx, qreal dy, int currentAxis) const;
        Q_INVOKABLE QPointF project(qreal dx, qreal dy, int axis) const;
        Q_INVOKABLE QPointF vector(int direction) const;
        Q_INVOKABLE qreal minDistance() const;

    private:
        mf::SwipeThresholds m_thresholds;
};
