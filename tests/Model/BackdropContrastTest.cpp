#include <gtest/gtest.h>

#include <QList>

#include "BackdropContrast.h"

using mf::BackdropTone;
using mf::meanLuma;
using mf::pixelLuma;
using mf::readableLightness;
using mf::sampleStripsBeside;
using mf::ToneThresholds;

namespace {

QList<quint32> solid(quint32 argb, qsizetype count) {
    return QList<quint32>(count, argb);
}

} // namespace

TEST(BackdropContrastTest, PixelLumaSpansBlackToWhite) {
    EXPECT_DOUBLE_EQ(pixelLuma(0xFF000000u), 0.0);
    EXPECT_DOUBLE_EQ(pixelLuma(0xFFFFFFFFu), 1.0);
    EXPECT_NEAR(pixelLuma(0xFF808080u), 0.502, 0.01);
    // Green weighs more than blue.
    EXPECT_GT(pixelLuma(0xFF00FF00u), pixelLuma(0xFF0000FFu));
}

TEST(BackdropContrastTest, MeanLumaSkipsTransparentPixelsAndEmptyInput) {
    EXPECT_DOUBLE_EQ(meanLuma(nullptr, 10), -1.0);
    const QList<quint32> none;
    EXPECT_DOUBLE_EQ(meanLuma(none.constData(), none.size()), -1.0);

    QList<quint32> mixed = solid(0xFFFFFFFFu, 2);
    mixed.append(solid(0xFF000000u, 2));
    EXPECT_NEAR(meanLuma(mixed.constData(), mixed.size()), 0.5, 1e-9);

    mixed.append(solid(0x00FFFFFFu, 100)); // Transparent white must not count.
    EXPECT_NEAR(meanLuma(mixed.constData(), mixed.size()), 0.5, 1e-9);

    const QList<quint32> onlyTransparent = solid(0x00FFFFFFu, 4);
    EXPECT_DOUBLE_EQ(meanLuma(onlyTransparent.constData(), onlyTransparent.size()), -1.0);
}

TEST(BackdropContrastTest, ToneStartsDarkAndUnknown) {
    BackdropTone tone;
    EXPECT_FALSE(tone.known());
    EXPECT_FALSE(tone.light());
    EXPECT_LT(tone.luma(), 0.0);
    EXPECT_FALSE(tone.update(-1.0));
    EXPECT_FALSE(tone.known());
}

TEST(BackdropContrastTest, ToneFlipsWithHysteresis) {
    BackdropTone tone(ToneThresholds{0.4, 0.6, 1.0});
    EXPECT_FALSE(tone.update(0.5)); // In the dead band. Stays dark.
    EXPECT_TRUE(tone.known());
    EXPECT_FALSE(tone.light());

    EXPECT_TRUE(tone.update(0.9));
    EXPECT_TRUE(tone.light());

    EXPECT_FALSE(tone.update(0.5)); // Dead band again. Stays light.
    EXPECT_TRUE(tone.light());

    EXPECT_TRUE(tone.update(0.1));
    EXPECT_FALSE(tone.light());
}

TEST(BackdropContrastTest, ToneSmoothsSingleSpikes) {
    BackdropTone tone(ToneThresholds{0.4, 0.6, 0.25});
    tone.update(0.1);
    // One bright frame moves the smoothed value only a quarter of the way.
    EXPECT_FALSE(tone.update(1.0));
    EXPECT_NEAR(tone.luma(), 0.325, 1e-9);
    EXPECT_FALSE(tone.light());
    // A steady bright backdrop wins after a few samples.
    bool flipped = false;
    for(int i = 0; i < 10 && !flipped; ++i)
        flipped = tone.update(1.0);
    EXPECT_TRUE(flipped);
    EXPECT_TRUE(tone.light());
}

TEST(BackdropContrastTest, ToneClampsAndResets) {
    BackdropTone tone(ToneThresholds{0.4, 0.6, 1.0});
    tone.update(5.0);
    EXPECT_DOUBLE_EQ(tone.luma(), 1.0);
    EXPECT_TRUE(tone.light());
    tone.reset();
    EXPECT_FALSE(tone.known());
    EXPECT_FALSE(tone.light());
}

TEST(BackdropContrastTest, ReadableLightnessPushesAwayFromTheBackdrop) {
    EXPECT_DOUBLE_EQ(readableLightness(1.0, true), mf::kDarkForegroundLightness);
    EXPECT_DOUBLE_EQ(readableLightness(0.0, false), mf::kLightForegroundLightness);
    // Already readable colors are kept as they are.
    EXPECT_DOUBLE_EQ(readableLightness(0.3, true), 0.3);
    EXPECT_DOUBLE_EQ(readableLightness(0.7, false), 0.7);
    EXPECT_DOUBLE_EQ(readableLightness(0.5, true), 0.5);
    EXPECT_DOUBLE_EQ(readableLightness(0.5, false), 0.5);
}

TEST(BackdropContrastTest, StripsSitBesideTheWidgetAtWidgetHeight) {
    const QRect widget(1000, 1040, 220, 40);
    const QRect screen(0, 0, 1920, 1080);
    const QList<QRect> strips = sampleStripsBeside(widget, screen, 24, 6);
    ASSERT_EQ(strips.size(), 2);
    EXPECT_EQ(strips[0], QRect(970, 1040, 24, 40));
    EXPECT_EQ(strips[1], QRect(1226, 1040, 24, 40));
    for(const QRect& strip : strips)
        EXPECT_FALSE(strip.intersects(widget));
}

TEST(BackdropContrastTest, StripsAreClippedToBoundsAndDroppedWhenOffScreen) {
    const QRect screen(0, 0, 1920, 1080);
    const QList<QRect> left = sampleStripsBeside(QRect(4, 1040, 220, 40), screen, 24, 6);
    ASSERT_EQ(left.size(), 1);
    EXPECT_EQ(left[0], QRect(230, 1040, 24, 40));

    const QList<QRect> partial = sampleStripsBeside(QRect(20, 1040, 220, 40), screen, 24, 6);
    ASSERT_EQ(partial.size(), 2);
    EXPECT_EQ(partial[0], QRect(0, 1040, 14, 40));

    EXPECT_TRUE(sampleStripsBeside(QRect(), screen, 24, 6).isEmpty());
    EXPECT_TRUE(sampleStripsBeside(QRect(10, 10, 20, 20), screen, 0, 6).isEmpty());
    // Null bounds mean no clipping.
    EXPECT_EQ(sampleStripsBeside(QRect(4, 1040, 220, 40), QRect(), 24, 6).size(), 2);
}

TEST(BackdropContrastTest, StripsAvoidSiblingWidgets) {
    const QRect media(1000, 1040, 220, 40);
    const QRect peripherals(1228, 1040, 90, 40); // 8 px to the right of media.
    const QRect screen(0, 0, 1920, 1080);
    const QList<QRect> strips = sampleStripsBeside(media, screen, 24, 6, {peripherals, QRect()});
    ASSERT_EQ(strips.size(), 1);
    EXPECT_EQ(strips[0], QRect(970, 1040, 24, 40));
}
