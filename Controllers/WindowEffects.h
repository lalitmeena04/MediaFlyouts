#pragma once

#include <QObject>

class QWindow;

// Applies the Windows 11 acrylic/mica system backdrop to a window (DWM).
class WindowEffects : public QObject {
        Q_OBJECT

    public:
        explicit WindowEffects(QObject* parent = nullptr);

        // enable = translucent acrylic backdrop; disable = solid.
        Q_INVOKABLE void applyAcrylic(QWindow* window, bool enable);
        // toggle the dark title bar / frame to match the theme
        Q_INVOKABLE void setDarkFrame(QWindow* window, bool dark);
};
