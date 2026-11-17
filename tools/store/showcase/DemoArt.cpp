#include "DemoArt.h"

#include <QFont>
#include <QFontDatabase>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>

#include <array>
#include <cmath>

namespace mf::demo {

namespace {

struct Palette {
        QColor top;
        QColor bottom;
        QColor spark;
};

const std::array<Palette, 6> kPalettes = {{
    {QColor("#12B5CB"), QColor("#102A54"), QColor("#FFB38A")},
    {QColor("#FF3D9A"), QColor("#2D1B8F"), QColor("#6FE3FF")},
    {QColor("#FFC533"), QColor("#E2491B"), QColor("#4B1FA8")},
    {QColor("#7BEFA2"), QColor("#14506E"), QColor("#E9FFE1")},
    {QColor("#D9A7FF"), QColor("#4A148C"), QColor("#FFA726")},
    {QColor("#FF5C7A"), QColor("#0A3A4A"), QColor("#FFD86B")},
}};

struct AppTile {
        QColor top;
        QColor bottom;
        ushort glyph;
};

// Glyphs from the bundled Fluent icon font: music note, play, gamepad, headset.
const std::array<AppTile, 4> kAppTiles = {{
    {QColor("#FF7AB6"), QColor("#C2185B"), 0xE863},
    {QColor("#6FB7FF"), QColor("#1E63D6"), 0xF610},
    {QColor("#8CE99A"), QColor("#2B8A3E"), 0xEEB9},
    {QColor("#B197FC"), QColor("#5F3DC4"), 0xF478},
}};

void paintBackground(QPainter& p, const Palette& palette, int size) {
    QLinearGradient gradient(0, 0, size * 0.35, size);
    gradient.setColorAt(0.0, palette.top);
    gradient.setColorAt(1.0, palette.bottom);
    p.fillRect(0, 0, size, size, gradient);
}

// Sun over layered hills.
void paintHills(QPainter& p, const Palette& palette, int size) {
    QRadialGradient sun(size * 0.62, size * 0.36, size * 0.26);
    QColor core = palette.spark;
    QColor edge = palette.spark;
    edge.setAlpha(0);
    sun.setColorAt(0.0, core);
    sun.setColorAt(0.72, core);
    sun.setColorAt(1.0, edge);
    p.setPen(Qt::NoPen);
    p.setBrush(sun);
    p.drawEllipse(QPointF(size * 0.62, size * 0.36), size * 0.26, size * 0.26);

    for(int layer = 0; layer < 3; ++layer) {
        const double base = size * (0.60 + layer * 0.13);
        const double swing = size * (0.07 - layer * 0.015);
        QPainterPath hill;
        hill.moveTo(0, size);
        for(int x = 0; x <= size; x += 4)
            hill.lineTo(x, base + swing * std::sin((x / double(size)) * 6.283 * (1.0 + layer * 0.35) + layer * 1.7));
        hill.lineTo(size, size);
        hill.closeSubpath();
        p.setBrush(palette.bottom.darker(120 + layer * 45));
        p.drawPath(hill);
    }
}

// Rings spreading from one corner.
void paintRings(QPainter& p, const Palette& palette, int size) {
    const QPointF center(size * 0.22, size * 0.78);
    p.setBrush(Qt::NoBrush);
    for(int ring = 1; ring <= 9; ++ring) {
        QColor color = palette.spark;
        color.setAlphaF(std::max(0.08, 0.85 - ring * 0.09));
        p.setPen(QPen(color, size * 0.022));
        p.drawEllipse(center, size * 0.11 * ring, size * 0.11 * ring);
    }
}

// Diagonal bars.
void paintBars(QPainter& p, const Palette& palette, int size) {
    p.save();
    p.translate(size / 2.0, size / 2.0);
    p.rotate(-38);
    p.setPen(Qt::NoPen);
    for(int bar = -5; bar <= 5; ++bar) {
        QColor color = bar % 2 == 0 ? palette.spark : palette.top.lighter(130);
        color.setAlphaF(0.20 + 0.06 * std::abs(bar % 3));
        const double length = size * (0.5 + 0.12 * ((bar + 5) % 4));
        p.setBrush(color);
        p.drawRoundedRect(QRectF(-length / 2, bar * size * 0.105, length, size * 0.06), size * 0.03, size * 0.03);
    }
    p.restore();
}

// Soft discs of different sizes.
void paintDiscs(QPainter& p, const Palette& palette, int size) {
    static const std::array<std::array<double, 3>, 7> discs = {{
        {0.24, 0.30, 0.20}, {0.70, 0.24, 0.13}, {0.55, 0.62, 0.27}, {0.16, 0.76, 0.11},
        {0.86, 0.72, 0.16}, {0.40, 0.14, 0.07}, {0.82, 0.46, 0.06},
    }};
    p.setPen(Qt::NoPen);
    int n = 0;
    for(const auto& disc : discs) {
        QColor color = n % 2 == 0 ? palette.spark : palette.top.lighter(140);
        color.setAlphaF(0.30 + 0.07 * (n % 4));
        p.setBrush(color);
        p.drawEllipse(QPointF(size * disc[0], size * disc[1]), size * disc[2], size * disc[2]);
        ++n;
    }
}

QString iconFontFamily() {
    static const QString family = [] {
        const int id =
            QFontDatabase::addApplicationFont(QStringLiteral(":/qt/qml/MediaFlyouts/assets/fonts/FluentSystemIcons-Filled.ttf"));
        const QStringList families = QFontDatabase::applicationFontFamilies(id);
        return families.isEmpty() ? QString() : families.first();
    }();
    return family;
}

} // namespace

int coverCount() {
    return static_cast<int>(kPalettes.size());
}

QImage coverArt(int index, int size) {
    const int n = coverCount();
    const int wrapped = ((index % n) + n) % n;
    const Palette& palette = kPalettes[static_cast<size_t>(wrapped)];

    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    paintBackground(p, palette, size);
    switch(wrapped % 4) {
    case 0: paintHills(p, palette, size); break;
    case 1: paintRings(p, palette, size); break;
    case 2: paintBars(p, palette, size); break;
    default: paintDiscs(p, palette, size); break;
    }
    return image;
}

int appIconCount() {
    return static_cast<int>(kAppTiles.size());
}

QImage appIcon(int index, int size) {
    const int n = appIconCount();
    const AppTile& tile = kAppTiles[static_cast<size_t>(((index % n) + n) % n)];

    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    QLinearGradient gradient(0, 0, 0, size);
    gradient.setColorAt(0.0, tile.top);
    gradient.setColorAt(1.0, tile.bottom);
    p.setPen(Qt::NoPen);
    p.setBrush(gradient);
    p.drawRoundedRect(QRectF(0, 0, size, size), size * 0.24, size * 0.24);

    const QString family = iconFontFamily();
    if(!family.isEmpty()) {
        QFont font(family);
        font.setPixelSize(static_cast<int>(size * 0.56));
        p.setFont(font);
        p.setPen(Qt::white);
        p.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, QString(QChar(tile.glyph)));
    }
    return image;
}

} // namespace mf::demo
