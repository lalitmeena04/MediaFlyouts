#pragma once

#include <vector>

namespace mf {

// Arrays re and im need same power-of-two length.
void fft(std::vector<double>& re, std::vector<double>& im);

std::vector<double> magnitudeSpectrum(const std::vector<double>& samples);

std::vector<double> spectrumBands(const std::vector<double>& magnitudes, int bands);

std::vector<double> frequencyBars(const std::vector<double>& magnitudes, double sampleRate, int fftSize, int bars,
                                  double minFreq = 40.0, double maxFreq = 8000.0, double minDb = -48.0,
                                  double maxDb = -6.0);

} // namespace mf
