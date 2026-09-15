#pragma once

#include <QObject>

class MonitorMapper;

// Detect fullscreen game in foreground. Prevent flyout focus issues.
class FullscreenDetector : public QObject {
        Q_OBJECT

    public:
        explicit FullscreenDetector(const MonitorMapper* monitors, QObject* parent = nullptr);

        Q_INVOKABLE bool isLocked() const;
        // True for any fullscreen foreground app, including browsers and media players.
        Q_INVOKABLE bool isFullscreen(int monitor = -1) const;
        // True only for fullscreen apps using a 3D graphics API.
        Q_INVOKABLE bool isGraphicsFullscreen(int monitor = -1) const;
        Q_INVOKABLE bool shouldBlock(bool disableOnFullscreen, int monitor = -1) const;

    private:
        const MonitorMapper* m_monitors;
};
