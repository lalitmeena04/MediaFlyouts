#pragma once

#include <QObject>
#include <QPoint>

class Settings;
class MonitorMapper;

class FlyoutPlacement : public QObject {
        Q_OBJECT

    public:
        explicit FlyoutPlacement(Settings* settings, const MonitorMapper* monitors, QObject* parent = nullptr);

        Q_INVOKABLE QPoint position(int width, int height) const;

        Q_INVOKABLE QPoint positionAt(int width, int height, int positionIndex, int monitor) const;

    private:
        Settings* m_settings;
        const MonitorMapper* m_monitors;
};
