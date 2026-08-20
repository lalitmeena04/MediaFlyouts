#pragma once

#include <vector>

namespace mf {

// In-place iterative radix-2 Cooley-Tukey FFT. re/im must be the same length,
// and that length must be a power of two.
void fft(std::vector<double>& re, std::vector<double>& im);

// Magnitude spectrum (first size/2 bins) of a real signal. The input is
// truncated to the largest power of two that fits (min 2); returns {} if too short.
std::vector<double> magnitudeSpectrum(const std::vector<double>& samples);

// Collapse a magnitude spectrum into `bands` log-spaced, normalised (0..1) values
// suitable for a bar visualizer.
std::vector<double> spectrumBands(const std::vector<double>& magnitudes, int bands);

// Map a magnitude spectrum onto `bars` log-spaced frequency bars across
// [minFreq, maxFreq], with a per-bar treble boost and a dB -> intensity (0..1)
// mapping. Unlike spectrumBands this is an absolute scale, so silence stays near 0.
std::vector<double> frequencyBars(const std::vector<double>& magnitudes, double sampleRate, int fftSize, int bars,
                                  double minFreq = 40.0, double maxFreq = 8000.0, double minDb = -48.0,
                                  double maxDb = -6.0);

} // namespace mf
