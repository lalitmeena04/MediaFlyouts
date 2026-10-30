#pragma once

#include <QList>
#include <QRect>
#include <QtGlobal>

// Portable core behind TaskbarBackdrop. Decides whether pixels read as light
// or dark, and which foreground lightness stays readable on top of them.
namespace mf {

// Rec. 601 luma of one gamma-encoded 0xAARRGGBB pixel. 0 = black. 1 = white.
double pixelLuma(quint32 argb);

// Mean luma of a pixel block. Fully transparent pixels are skipped.
// Returns -1 when no pixel counted.
double meanLuma(const quint32* pixels, qsizetype count);

struct ToneThresholds {
        double darkBelow = 0.42;  // Tone flips to dark under this luma.
        double lightAbove = 0.55; // Tone flips to light above this luma.
        double smoothing = 0.5;   // Weight of the new sample. 1 = no smoothing.
};

// Tracks whether a backdrop reads as light. Samples are smoothed and the
// flip uses hysteresis, so a live wallpaper does not make the text flicker.
class BackdropTone {
    public:
        explicit BackdropTone(ToneThresholds thresholds = {});

        // Feeds one measured luma. Negative values are ignored.
        // Returns true when light() flipped.
        bool update(double luma);
        void reset();

        bool known() const;
        bool light() const;
        double luma() const;

    private:
        ToneThresholds m_thresholds;
        bool m_light = false;
        double m_luma = -1.0;
};

inline constexpr double kDarkForegroundLightness = 0.28;
inline constexpr double kLightForegroundLightness = 0.74;

// Target HSL lightness for a foreground color on the backdrop.
// Returns the input unchanged when it already reads well.
double readableLightness(double lightness, bool lightBackdrop);

// Rects of the same height as the widget, one on each side, `gap` away.
// Clipped to bounds. Strips that touch an avoid rect are dropped, so a
// sibling widget never ends up in the sample.
QList<QRect> sampleStripsBeside(const QRect& widget, const QRect& bounds, int stripWidth, int gap,
                                const QList<QRect>& avoid = {});

} // namespace mf
