#pragma once

#include <vector>

namespace mf {

// In-place iterative radix-2 Cooley-Tukey FFT.
// Arrays re and im need same power-of-two length.
void fft(std::vector<double>& re, std::vector<double>& im);

// Magnitude spectrum of real signal. Uses first size/2 bins.
// Truncate input to largest power of two. Return empty if too short.
std::vector<double> magnitudeSpectrum(const std::vector<double>& samples);

// Collapse magnitude spectrum into log-spaced normalized bands.
std::vector<double> spectrumBands(const std::vector<double>& magnitudes, int bands);

// Map magnitude spectrum to log-spaced frequency bars.
// Uses absolute scale. Silence stays near 0.
std::vector<double> frequencyBars(const std::vector<double>& magnitudes, double sampleRate, int fftSize, int bars,
                                  double minFreq = 40.0, double maxFreq = 8000.0, double minDb = -48.0,
                                  double maxDb = -6.0);

} // namespace mf
