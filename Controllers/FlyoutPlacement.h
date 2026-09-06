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

        // Top-left point for selected monitor and position.
        Q_INVOKABLE QPoint position(int width, int height) const;

        // Top-left point for explicit monitor and position.
        Q_INVOKABLE QPoint positionAt(int width, int height, int positionIndex, int monitor) const;

        // Human-readable monitor list for dashboard picker.
        Q_INVOKABLE QStringList monitorNames() const;

    private:
        Settings* m_settings;
};
