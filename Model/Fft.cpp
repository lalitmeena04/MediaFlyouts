#include "Fft.h"

#include <cmath>

namespace mf {

namespace {
constexpr double kPi = 3.14159265358979323846;

bool isPowerOfTwo(std::size_t n) {
    return n >= 2 && (n & (n - 1)) == 0;
}

std::size_t largestPowerOfTwo(std::size_t n) {
    std::size_t p = 1;
    while(p * 2 <= n)
        p *= 2;
    return p;
}
} // namespace

void fft(std::vector<double>& re, std::vector<double>& im) {
    const std::size_t n = re.size();
    if(n < 2 || re.size() != im.size() || !isPowerOfTwo(n))
        return;

    // bit-reversal permutation
    for(std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for(; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if(i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    for(std::size_t len = 2; len <= n; len <<= 1) {
        const double ang = -2.0 * kPi / static_cast<double>(len);
        const double wRe = std::cos(ang);
        const double wIm = std::sin(ang);
        for(std::size_t i = 0; i < n; i += len) {
            double curRe = 1.0;
            double curIm = 0.0;
            for(std::size_t k = 0; k < len / 2; ++k) {
                const std::size_t a = i + k;
                const std::size_t b = i + k + len / 2;
                const double tRe = re[b] * curRe - im[b] * curIm;
                const double tIm = re[b] * curIm + im[b] * curRe;
                re[b] = re[a] - tRe;
                im[b] = im[a] - tIm;
                re[a] += tRe;
                im[a] += tIm;
                const double nextRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = nextRe;
            }
        }
    }
}

std::vector<double> magnitudeSpectrum(const std::vector<double>& samples) {
    if(samples.size() < 2)
        return {};
    const std::size_t n = largestPowerOfTwo(samples.size());
    std::vector<double> re(samples.begin(), samples.begin() + static_cast<long>(n));
    std::vector<double> im(n, 0.0);
    fft(re, im);

    std::vector<double> mags(n / 2);
    for(std::size_t i = 0; i < n / 2; ++i)
        mags[i] = std::sqrt(re[i] * re[i] + im[i] * im[i]);
    return mags;
}

std::vector<double> spectrumBands(const std::vector<double>& magnitudes, int bands) {
    if(bands <= 0 || magnitudes.size() < 2)
        return std::vector<double>(bands > 0 ? bands : 0, 0.0);

    std::vector<double> out(bands, 0.0);
    const std::size_t bins = magnitudes.size();
    double peak = 1e-9;
    for(int b = 0; b < bands; ++b) {
        // log-spaced bin ranges so low frequencies aren't all squashed into one bar
        const double lo = std::pow(static_cast<double>(bins), static_cast<double>(b) / bands);
        const double hi = std::pow(static_cast<double>(bins), static_cast<double>(b + 1) / bands);
        std::size_t start = static_cast<std::size_t>(lo);
        std::size_t end = static_cast<std::size_t>(hi);
        if(end <= start)
            end = start + 1;
        if(end > bins)
            end = bins;
        double sum = 0.0;
        for(std::size_t i = start; i < end; ++i)
            sum += magnitudes[i];
        const double avg = sum / static_cast<double>(end - start);
        out[b] = avg;
        if(avg > peak)
            peak = avg;
    }
    for(double& v : out)
        v = v / peak;
    return out;
}

std::vector<double> frequencyBars(const std::vector<double>& magnitudes, double sampleRate, int fftSize, int bars,
                                  double minFreq, double maxFreq, double minDb, double maxDb) {
    std::vector<double> out(bars > 0 ? bars : 0, 0.0);
    if(bars <= 0 || magnitudes.empty() || sampleRate <= 0.0 || fftSize <= 0 || maxDb <= minDb)
        return out;

    const double binHz = sampleRate / static_cast<double>(fftSize);
    const int maxBin = static_cast<int>(magnitudes.size());
    const double norm = static_cast<double>(fftSize) * 0.5; // full-scale single-bin magnitude

    for(int i = 0; i < bars; ++i) {
        const double startFreq = minFreq * std::pow(maxFreq / minFreq, static_cast<double>(i) / bars);
        const double endFreq = minFreq * std::pow(maxFreq / minFreq, static_cast<double>(i + 1) / bars);
        int startBin = static_cast<int>(startFreq / binHz);
        int endBin = static_cast<int>(endFreq / binHz);
        if(startBin < 0)
            startBin = 0;
        if(endBin <= startBin)
            endBin = startBin + 1;
        if(endBin > maxBin)
            endBin = maxBin;
        if(startBin >= maxBin)
            break;

        double maxAmp = 0.0;
        for(int j = startBin; j < endBin; ++j)
            if(magnitudes[j] > maxAmp)
                maxAmp = magnitudes[j];

        const double progress = static_cast<double>(i) / bars;
        double amp = (maxAmp / norm) * (1.0 + progress * 4.0); // lift treble so it stays visible
        if(amp < 1e-4)
            amp = 1e-4;

        const double db = 20.0 * std::log10(amp);
        double intensity = (db - minDb) / (maxDb - minDb);
        out[i] = intensity < 0.0 ? 0.0 : (intensity > 1.0 ? 1.0 : intensity);
    }
    return out;
}

} // namespace mf
