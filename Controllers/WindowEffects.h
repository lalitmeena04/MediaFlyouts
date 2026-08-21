#pragma once

#include <QObject>

class QWindow;

// Small helper for Windows-specific window chrome tweaks (DWM).
class WindowEffects : public QObject {
        Q_OBJECT

    public:
        explicit WindowEffects(QObject* parent = nullptr);

        // toggle the dark title bar / frame to match the theme
        Q_INVOKABLE void setDarkFrame(QWindow* window, bool dark);
};
