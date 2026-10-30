#include "BackdropContrast.h"

namespace mf {

double pixelLuma(quint32 argb) {
    const double r = ((argb >> 16) & 0xFF) / 255.0;
    const double g = ((argb >> 8) & 0xFF) / 255.0;
    const double b = (argb & 0xFF) / 255.0;
    return 0.299 * r + 0.587 * g + 0.114 * b;
}

double meanLuma(const quint32* pixels, qsizetype count) {
    if(!pixels || count <= 0)
        return -1.0;
    double sum = 0.0;
    qsizetype counted = 0;
    for(qsizetype i = 0; i < count; ++i) {
        if(((pixels[i] >> 24) & 0xFF) == 0)
            continue;
        sum += pixelLuma(pixels[i]);
        ++counted;
    }
    return counted > 0 ? sum / counted : -1.0;
}

BackdropTone::BackdropTone(ToneThresholds thresholds) : m_thresholds(thresholds) {
}

bool BackdropTone::update(double luma) {
    if(luma < 0.0)
        return false;
    luma = qBound(0.0, luma, 1.0);
    if(m_luma < 0.0)
        m_luma = luma;
    else
        m_luma += (luma - m_luma) * qBound(0.0, m_thresholds.smoothing, 1.0);

    const bool was = m_light;
    if(m_light && m_luma < m_thresholds.darkBelow)
        m_light = false;
    else if(!m_light && m_luma > m_thresholds.lightAbove)
        m_light = true;
    return was != m_light;
}

void BackdropTone::reset() {
    m_light = false;
    m_luma = -1.0;
}

bool BackdropTone::known() const {
    return m_luma >= 0.0;
}

bool BackdropTone::light() const {
    return m_light;
}

double BackdropTone::luma() const {
    return m_luma;
}

double readableLightness(double lightness, bool lightBackdrop) {
    // A color near the backdrop tone gets pushed to the other side.
    if(lightBackdrop && lightness > 0.55)
        return kDarkForegroundLightness;
    if(!lightBackdrop && lightness < 0.45)
        return kLightForegroundLightness;
    return lightness;
}

QList<QRect> sampleStripsBeside(const QRect& widget, const QRect& bounds, int stripWidth, int gap,
                                const QList<QRect>& avoid) {
    QList<QRect> strips;
    if(widget.isEmpty() || stripWidth <= 0)
        return strips;

    const QRect left(widget.left() - gap - stripWidth, widget.top(), stripWidth, widget.height());
    const QRect right(widget.right() + 1 + gap, widget.top(), stripWidth, widget.height());
    for(const QRect& candidate : {left, right}) {
        const QRect clipped = bounds.isNull() ? candidate : candidate.intersected(bounds);
        if(clipped.isEmpty())
            continue;
        bool blocked = false;
        for(const QRect& other : avoid) {
            if(!other.isEmpty() && clipped.intersects(other)) {
                blocked = true;
                break;
            }
        }
        if(!blocked)
            strips.append(clipped);
    }
    return strips;
}

} // namespace mf
