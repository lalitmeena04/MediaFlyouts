#pragma once

#include <QObject>

// Detects a fullscreen video game in the foreground so flyouts don't steal focus and minimize it.
class FullscreenDetector : public QObject {
        Q_OBJECT

    public:
        explicit FullscreenDetector(QObject* parent = nullptr);

        Q_INVOKABLE bool isLocked() const;
        // True for any fullscreen foreground app, including browsers and media players.
        Q_INVOKABLE bool isFullscreen(int monitor = -1) const;
        // True only for fullscreen apps using a 3D graphics API.
        Q_INVOKABLE bool isGraphicsFullscreen(int monitor = -1) const;
        Q_INVOKABLE bool shouldBlock(bool disableOnFullscreen, int monitor = -1) const;
};
