#pragma once

#include <QObject>

// Detects a fullscreen video game in the foreground so flyouts don't steal focus and minimize it.
class FullscreenDetector : public QObject {
        Q_OBJECT

    public:
        explicit FullscreenDetector(QObject* parent = nullptr);

        Q_INVOKABLE bool isLocked() const;
        // monitor is a QGuiApplication screen index; -1 means any monitor (global check)
        Q_INVOKABLE bool isFullscreen(int monitor = -1) const;
        Q_INVOKABLE bool shouldBlock(bool disableOnFullscreen, int monitor = -1) const;
};
