#include "AudioCapture.h"

#include <QDateTime>
#include <QtGlobal>

#include <cmath>

#include <windows.h>

#include <audioclient.h>
#include <mmdeviceapi.h>
#include <mmreg.h>

#include "Fft.h"

namespace {
constexpr int kFftSize = 1024;
constexpr double kPi = 3.14159265358979323846;
} // namespace

AudioCapture::AudioCapture(int bandCount, QObject* parent)
    : QObject(parent), m_bandCount(bandCount > 0 ? bandCount : 24) {
    m_smoothed.assign(m_bandCount, 0.0);
    for(int i = 0; i < m_bandCount; ++i)
        m_bands.append(0.0);
    // ~60 fps polling keeps the loopback buffer drained and the bars smooth
    m_timer.setInterval(16);
    connect(&m_timer, &QTimer::timeout, this, &AudioCapture::poll);
}

AudioCapture::~AudioCapture() {
    stop();
}

QVariantList AudioCapture::bands() const {
    return m_bands;
}

bool AudioCapture::active() const {
    return m_active;
}

double AudioCapture::level() const {
    return m_level;
}

void AudioCapture::start() {
    if(m_active)
        return;
    if(!openDevice()) {
        releaseDevice();
        return;
    }
    m_active = true;
    emit activeChanged();
    m_timer.start();
}

bool AudioCapture::openDevice() {
    if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                               reinterpret_cast<void**>(&m_enumerator)))) {
        return false;
    }
    if(FAILED(m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device))) {
        return false;
    }
    if(FAILED(m_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&m_client)))) {
        return false;
    }
    if(FAILED(m_client->GetMixFormat(&m_format)) || !m_format) {
        return false;
    }

    m_channels = m_format->nChannels > 0 ? m_format->nChannels : 2;
    m_sampleRate = m_format->nSamplesPerSec > 0 ? static_cast<int>(m_format->nSamplesPerSec) : 48000;
    m_isFloat = false;
    if(m_format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT && m_format->wBitsPerSample == 32) {
        m_isFloat = true;
    } else if(m_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        auto* ext = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(m_format);
        if(ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT && m_format->wBitsPerSample == 32)
            m_isFloat = true;
    }

    if(FAILED(m_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, m_format, nullptr))) {
        return false;
    }
    if(FAILED(m_client->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(&m_capture)))) {
        return false;
    }
    if(FAILED(m_client->Start())) {
        return false;
    }

    m_accum.clear();
    return true;
}

void AudioCapture::stop() {
    if(!m_active && !m_client) {
        return;
    }
    m_timer.stop();
    if(m_client)
        m_client->Stop();
    releaseDevice();
    if(m_active) {
        m_active = false;
        emit activeChanged();
    }
    publishSilence();
}

void AudioCapture::releaseDevice() {
    if(m_capture) {
        m_capture->Release();
        m_capture = nullptr;
    }
    if(m_format) {
        CoTaskMemFree(m_format);
        m_format = nullptr;
    }
    if(m_client) {
        m_client->Release();
        m_client = nullptr;
    }
    if(m_device) {
        m_device->Release();
        m_device = nullptr;
    }
    if(m_enumerator) {
        m_enumerator->Release();
        m_enumerator = nullptr;
    }
}

void AudioCapture::publishSilence() {
    for(int i = 0; i < m_bandCount; ++i)
        m_bands[i] = 0.0;
    std::fill(m_smoothed.begin(), m_smoothed.end(), 0.0);
    m_bassAvg = 0.0;
    if(m_level != 0.0) {
        m_level = 0.0;
        emit levelChanged();
    }
    emit bandsChanged();
}

void AudioCapture::handleDeviceLoss() {
    // a WASAPI call failed (endpoint invalidated / default device changed);
    // drop the stale device so poll() re-acquires the current endpoint
    if(m_client)
        m_client->Stop();
    releaseDevice();
    m_accum.clear();
    publishSilence();
}

void AudioCapture::tryRecover() {
    // re-acquire the default render endpoint after a device loss, throttled so a
    // missing/failing device doesn't spin the audio stack every poll tick
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if(now - m_lastRecoverMs < 750)
        return;
    m_lastRecoverMs = now;
    if(!openDevice())
        releaseDevice();
}

void AudioCapture::poll() {
    if(!m_capture) {
        if(m_active)
            tryRecover();
        return;
    }
    if(!m_isFloat)
        return;

    UINT32 packet = 0;
    if(FAILED(m_capture->GetNextPacketSize(&packet))) {
        handleDeviceLoss();
        return;
    }

    while(packet != 0) {
        BYTE* data = nullptr;
        UINT32 frames = 0;
        DWORD flags = 0;
        if(FAILED(m_capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr))) {
            handleDeviceLoss();
            return;
        }

        const bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0;
        const auto* samples = reinterpret_cast<const float*>(data);
        for(UINT32 f = 0; f < frames; ++f) {
            double mono = 0.0;
            if(!silent && samples) {
                for(int c = 0; c < m_channels; ++c)
                    mono += samples[f * m_channels + c];
                mono /= m_channels;
            }
            m_accum.push_back(mono);
        }
        m_capture->ReleaseBuffer(frames);
        if(FAILED(m_capture->GetNextPacketSize(&packet))) {
            handleDeviceLoss();
            return;
        }
    }

    if(static_cast<int>(m_accum.size()) < kFftSize)
        return;

    // use the most recent window and apply a Hann taper to reduce spectral leakage
    std::vector<double> window(m_accum.end() - kFftSize, m_accum.end());
    m_accum.clear();
    for(int i = 0; i < kFftSize; ++i)
        window[i] *= 0.5 * (1.0 - std::cos(2.0 * kPi * i / (kFftSize - 1)));

    const std::vector<double> mags = mf::magnitudeSpectrum(window);
    std::vector<double> raw = mf::frequencyBars(mags, m_sampleRate, kFftSize, m_bandCount);

    bool changed = false;
    for(int i = 0; i < m_bandCount; ++i) {
        // rise quickly, fall slowly for a natural-looking meter
        const double target = raw[i];
        if(target > m_smoothed[i])
            m_smoothed[i] = target;
        else
            m_smoothed[i] = m_smoothed[i] * 0.82 + target * 0.18;
        const double rounded = std::round(m_smoothed[i] * 1000.0) / 1000.0;
        if(!qFuzzyCompare(rounded + 1.0, m_bands[i].toDouble() + 1.0)) {
            m_bands[i] = rounded;
            changed = true;
        }
    }
    if(changed)
        emit bandsChanged();

    // overall level (particle intensity) and a simple bass-energy beat detector
    double sum = 0.0;
    const int bassCount = std::max(1, m_bandCount / 4);
    double bass = 0.0;
    for(int i = 0; i < m_bandCount; ++i) {
        sum += raw[i];
        if(i < bassCount)
            bass += raw[i];
    }
    const double lvl = sum / m_bandCount;
    bass /= bassCount;
    if(!qFuzzyCompare(lvl + 1.0, m_level + 1.0)) {
        m_level = lvl;
        emit levelChanged();
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if(bass > 0.16 && bass > m_bassAvg * 1.35 && (now - m_lastBeatMs) > 180) {
        m_lastBeatMs = now;
        emit beat();
    }
    m_bassAvg = m_bassAvg * 0.9 + bass * 0.1;
}
