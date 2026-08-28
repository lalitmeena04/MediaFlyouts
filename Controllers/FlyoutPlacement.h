#pragma once

#include <QObject>
#include <QPoint>
#include <QStringList>

class Settings;

// Resolves flyout screen placement from the position + monitor settings, for QML to consume.
class FlyoutPlacement : public QObject {
        Q_OBJECT

    public:
        explicit FlyoutPlacement(Settings* settings, QObject* parent = nullptr);

        // top-left point for a flyout of the given size on the selected monitor and position
        Q_INVOKABLE QPoint position(int width, int height) const;

        // same, for a flyout that has its own position/monitor rather than the shared one
        Q_INVOKABLE QPoint positionAt(int width, int height, int positionIndex, int monitor) const;

        // human-readable list of the connected monitors, for the dashboard picker
        Q_INVOKABLE QStringList monitorNames() const;

    private:
        Settings* m_settings;
};
