#pragma once

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointer>

#include "BackdropContrast.h"

class QWindow;

// Reads the taskbar pixels beside a docked widget and reports whether they
// read as light or dark. The widget's own pixels are never sampled, so the
// widget can stay visible in screenshots and recordings.
class TaskbarBackdrop : public QObject {
        Q_OBJECT

        Q_PROPERTY(bool known READ known NOTIFY changed)
        Q_PROPERTY(bool light READ light NOTIFY changed)
        Q_PROPERTY(double luminance READ luminance NOTIFY changed)

    public:
        explicit TaskbarBackdrop(QObject* parent = nullptr);

        bool known() const;
        bool light() const;
        double luminance() const;

        // Samples the screen on both sides of the window. Other windows passed
        // here before are kept out of the sample.
        Q_INVOKABLE void sample(QWindow* window);

        // Returns the color with a lightness that stays readable on the backdrop.
        Q_INVOKABLE QColor readable(const QColor& color, bool lightBackdrop) const;

        // Feeds one measured luma. Test seam and shared tail of sample().
        void applyLuma(double luma);

    signals:
        void changed();

    private:
        mf::BackdropTone m_tone;
        QList<QPointer<QWindow>> m_known;
};
