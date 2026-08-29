#pragma once

#include <QObject>

// Detects a fullscreen video game in the foreground so flyouts don't steal focus and minimize it.
class FullscreenDetector : public QObject {
        Q_OBJECT

    public:
        explicit FullscreenDetector(QObject* parent = nullptr);

        Q_INVOKABLE bool isLocked() const;
        Q_INVOKABLE bool isFullscreen() const;
        // lock always blocks flyouts; fullscreen only blocks when the user setting is on
        Q_INVOKABLE bool shouldBlock(bool disableOnFullscreen) const;
};
