#pragma once

#include <QObject>

class QWindow;

// Applies the Windows 11 acrylic/mica system backdrop to a window (DWM).
class WindowEffects : public QObject {
        Q_OBJECT

    public:
        explicit WindowEffects(QObject* parent = nullptr);

        // mode: 0 = none (solid), 1 = acrylic (transient), 2 = mica (main window)
        Q_INVOKABLE void applyBackdrop(QWindow* window, int mode);
        // toggle the dark title bar / frame to match the theme
        Q_INVOKABLE void setDarkFrame(QWindow* window, bool dark);
};
