#include "TaskbarBackdrop.h"

#include <QWindow>
#include <windows.h>

#include "Logging.h"

namespace {

constexpr int kStripWidth = 24; // Logical pixels.
constexpr int kStripGap = 6;    // Logical pixels.

QRect nativeRect(HWND hwnd) {
    RECT r{};
    if(!hwnd || !GetWindowRect(hwnd, &r))
        return QRect();
    return QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

QRect monitorRect(HWND hwnd) {
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if(!monitor || !GetMonitorInfoW(monitor, &info))
        return QRect();
    const RECT& r = info.rcMonitor;
    return QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

// Copies one screen rect into a top-down 32-bit buffer. Returns false when GDI refused.
bool grabScreen(const QRect& rect, QList<quint32>& pixels) {
    pixels.clear();
    if(rect.isEmpty())
        return false;

    HDC screen = GetDC(nullptr);
    if(!screen)
        return false;
    HDC memory = CreateCompatibleDC(screen);
    if(!memory) {
        ReleaseDC(nullptr, screen);
        return false;
    }

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = rect.width();
    info.bmiHeader.biHeight = -rect.height();
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    bool ok = false;
    if(bitmap && bits) {
        HGDIOBJ previous = SelectObject(memory, bitmap);
        ok = BitBlt(memory, 0, 0, rect.width(), rect.height(), screen, rect.x(), rect.y(), SRCCOPY) != 0;
        GdiFlush();
        if(ok) {
            const qsizetype count = static_cast<qsizetype>(rect.width()) * rect.height();
            const quint32* src = static_cast<const quint32*>(bits);
            pixels.resize(count);
            for(qsizetype i = 0; i < count; ++i)
                pixels[i] = src[i] | 0xFF000000u; // BitBlt leaves alpha undefined.
        }
        SelectObject(memory, previous);
    }
    if(bitmap)
        DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(nullptr, screen);
    return ok;
}

} // namespace

TaskbarBackdrop::TaskbarBackdrop(QObject* parent) : QObject(parent) {
}

bool TaskbarBackdrop::known() const {
    return m_tone.known();
}

bool TaskbarBackdrop::light() const {
    return m_tone.light();
}

double TaskbarBackdrop::luminance() const {
    return m_tone.luma();
}

void TaskbarBackdrop::sample(QWindow* window) {
    if(!window)
        return;
    if(!m_known.contains(window))
        m_known.append(window);

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    const QRect widget = nativeRect(hwnd);
    if(widget.isEmpty()) {
        TRC_DEBUG("TaskbarBackdrop: window has no native rect yet");
        return;
    }

    QList<QRect> avoid;
    for(auto it = m_known.begin(); it != m_known.end();) {
        QWindow* other = *it;
        if(!other) {
            it = m_known.erase(it);
            continue;
        }
        if(other != window && other->isVisible())
            avoid.append(nativeRect(reinterpret_cast<HWND>(other->winId())));
        ++it;
    }

    const qreal dpr = window->devicePixelRatio() > 0 ? window->devicePixelRatio() : 1.0;
    const QList<QRect> strips = mf::sampleStripsBeside(widget, monitorRect(hwnd), qRound(kStripWidth * dpr),
                                                       qRound(kStripGap * dpr), avoid);
    if(strips.isEmpty()) {
        TRC_DEBUG("TaskbarBackdrop: no free strip beside widget ({},{} {}x{})", widget.x(), widget.y(),
                  widget.width(), widget.height());
        return;
    }

    double sum = 0.0;
    qsizetype counted = 0;
    QList<quint32> pixels;
    for(const QRect& strip : strips) {
        if(!grabScreen(strip, pixels)) {
            TRC_DEBUG("TaskbarBackdrop: BitBlt failed for strip ({},{} {}x{})", strip.x(), strip.y(), strip.width(),
                      strip.height());
            continue;
        }
        const double luma = mf::meanLuma(pixels.constData(), pixels.size());
        if(luma < 0.0)
            continue;
        sum += luma * pixels.size();
        counted += pixels.size();
    }
    if(counted == 0)
        return;
    applyLuma(sum / counted);
}

QColor TaskbarBackdrop::readable(const QColor& color, bool lightBackdrop) const {
    if(!color.isValid())
        return color;
    float h = 0, s = 0, l = 0, a = 1;
    color.getHslF(&h, &s, &l, &a);
    const double target = mf::readableLightness(l, lightBackdrop);
    if(qFuzzyCompare(target, static_cast<double>(l)))
        return color;
    QColor out;
    out.setHslF(h < 0 ? 0.0f : h, s, static_cast<float>(target), a);
    return out;
}

void TaskbarBackdrop::applyLuma(double luma) {
    const double before = m_tone.luma();
    const bool wasKnown = m_tone.known();
    const bool flipped = m_tone.update(luma);
    if(!wasKnown && m_tone.known())
        TRC_INFO("TaskbarBackdrop: first sample luma {:.2f}, reads {}", m_tone.luma(),
                 m_tone.light() ? "light" : "dark");
    else if(flipped)
        TRC_INFO("TaskbarBackdrop: backdrop now reads {} (luma {:.2f})", m_tone.light() ? "light" : "dark",
                 m_tone.luma());
    // Tiny drifts of a live wallpaper must not wake every QML binding.
    if(flipped || qAbs(m_tone.luma() - before) >= 0.01)
        emit changed();
}
