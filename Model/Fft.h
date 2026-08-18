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

} // namespace mf
