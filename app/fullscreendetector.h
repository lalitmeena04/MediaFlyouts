#pragma once

#include <QObject>

// Detects an exclusive-D3D / fullscreen foreground app so flyouts don't steal focus and minimize it.
class FullscreenDetector : public QObject {
        Q_OBJECT

    public:
        explicit FullscreenDetector(QObject* parent = nullptr);

        Q_INVOKABLE bool isActive() const;
};
