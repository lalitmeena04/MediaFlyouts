#include "Fft.h"

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

namespace {
constexpr double kPi = 3.14159265358979323846;
}

TEST(FftTest, PureSinePeaksAtItsBin) {
    const int n = 64;
    const int bin = 8;
    std::vector<double> samples(n);
    for(int i = 0; i < n; ++i)
        samples[i] = std::sin(2.0 * kPi * bin * i / n);

    const std::vector<double> mags = mf::magnitudeSpectrum(samples);
    ASSERT_EQ(mags.size(), static_cast<std::size_t>(n / 2));

    int peak = 0;
    for(std::size_t i = 1; i < mags.size(); ++i)
        if(mags[i] > mags[peak])
            peak = static_cast<int>(i);
    EXPECT_EQ(peak, bin);
}

TEST(FftTest, DcSignalHasZeroHigherBins) {
    std::vector<double> samples(16, 1.0);
    const std::vector<double> mags = mf::magnitudeSpectrum(samples);
    ASSERT_FALSE(mags.empty());
    EXPECT_GT(mags[0], 1.0);
    for(std::size_t i = 1; i < mags.size(); ++i)
        EXPECT_NEAR(mags[i], 0.0, 1e-6);
}

TEST(FftTest, SpectrumBandsAreNormalised) {
    std::vector<double> mags(64, 0.0);
    mags[40] = 5.0;
    const std::vector<double> bands = mf::spectrumBands(mags, 16);
    ASSERT_EQ(bands.size(), static_cast<std::size_t>(16));
    double maxV = 0.0;
    for(double v : bands) {
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
        maxV = std::max(maxV, v);
    }
    EXPECT_NEAR(maxV, 1.0, 1e-9);
}

TEST(FftTest, TooShortInputReturnsEmpty) {
    EXPECT_TRUE(mf::magnitudeSpectrum(std::vector<double>{1.0}).empty());
}

TEST(FftTest, FrequencyBarsSilenceStaysLow) {
    std::vector<double> mags(512, 0.0);
    const std::vector<double> bars = mf::frequencyBars(mags, 48000.0, 1024, 16);
    ASSERT_EQ(bars.size(), static_cast<std::size_t>(16));
    for(double v : bars) {
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
        EXPECT_LT(v, 0.05); // near-silence must not light the bars
    }
}

TEST(FftTest, FrequencyBarsLightBarAtToneFrequency) {
    const int fftSize = 1024;
    const double sampleRate = 48000.0;
    std::vector<double> mags(fftSize / 2, 0.0);
    // a strong component around 1 kHz -> bin ~ 1000 / (48000/1024) ~ 21
    const int bin = static_cast<int>(1000.0 / (sampleRate / fftSize));
    mags[bin] = fftSize * 0.5; // full-scale magnitude

    const int bars = 16;
    const std::vector<double> out = mf::frequencyBars(mags, sampleRate, fftSize, bars);
    ASSERT_EQ(out.size(), static_cast<std::size_t>(bars));
    double maxV = 0.0;
    for(double v : out)
        maxV = std::max(maxV, v);
    EXPECT_GT(maxV, 0.5); // the tone should drive at least one bar high
}
