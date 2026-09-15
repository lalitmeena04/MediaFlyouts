#pragma once

#include <QObject>
#include <QPoint>

class Settings;
class MonitorMapper;

// Resolves flyout screen placement from the position + monitor settings, for QML to consume.
class FlyoutPlacement : public QObject {
        Q_OBJECT

    public:
        explicit FlyoutPlacement(Settings* settings, const MonitorMapper* monitors, QObject* parent = nullptr);

        // Top-left point for selected monitor and position.
        Q_INVOKABLE QPoint position(int width, int height) const;

        // Top-left point for explicit monitor and position.
        Q_INVOKABLE QPoint positionAt(int width, int height, int positionIndex, int monitor) const;

    private:
        Settings* m_settings;
        const MonitorMapper* m_monitors;
};
