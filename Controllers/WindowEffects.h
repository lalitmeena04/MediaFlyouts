#pragma once

#include <QObject>

class QWindow;

class WindowEffects : public QObject {
        Q_OBJECT

    public:
        explicit WindowEffects(QObject* parent = nullptr);

        Q_INVOKABLE void setDarkFrame(QWindow* window, bool dark);

        Q_INVOKABLE void pinTopmost(QWindow* window);
};
