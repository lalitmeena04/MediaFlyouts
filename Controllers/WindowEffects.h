#pragma once

#include <QObject>

class QWindow;

// Helper for Windows window effects.
class WindowEffects : public QObject {
        Q_OBJECT

    public:
        explicit WindowEffects(QObject* parent = nullptr);

        // Match the title bar to the theme.
        Q_INVOKABLE void setDarkFrame(QWindow* window, bool dark);

        // Move above other topmost windows. Do not take focus.
        Q_INVOKABLE void pinTopmost(QWindow* window);
};
