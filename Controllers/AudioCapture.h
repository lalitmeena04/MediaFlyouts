#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>

#include <vector>

struct IMMDeviceEnumerator;
struct IMMDevice;
struct IAudioClient;
struct IAudioCaptureClient;
struct tWAVEFORMATEX;

// Captures the default render endpoint in WASAPI loopback mode and turns the
// signal into a set of normalised frequency-band levels for a bar visualizer.
class AudioCapture : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList bands READ bands NOTIFY bandsChanged)
        Q_PROPERTY(bool active READ active NOTIFY activeChanged)
        Q_PROPERTY(double level READ level NOTIFY levelChanged)

    public:
        explicit AudioCapture(int bandCount = 24, QObject* parent = nullptr);
        ~AudioCapture() override;

        QVariantList bands() const;
        bool active() const;
        double level() const;

    public slots:
        void start();
        void stop();

    signals:
        void bandsChanged();
        void activeChanged();
        void levelChanged();
        // emitted on a detected bass beat, drives the particle visualizer
        void beat();

    private:
        void poll();
        void releaseDevice();
        void publishSilence();

        int m_bandCount = 24;
        QTimer m_timer;
        QVariantList m_bands;
        std::vector<double> m_smoothed;
        std::vector<double> m_accum;

        IMMDeviceEnumerator* m_enumerator = nullptr;
        IMMDevice* m_device = nullptr;
        IAudioClient* m_client = nullptr;
        IAudioCaptureClient* m_capture = nullptr;
        tWAVEFORMATEX* m_format = nullptr;
        int m_channels = 2;
        int m_sampleRate = 48000;
        bool m_isFloat = false;
        bool m_active = false;
        double m_level = 0.0;
        double m_bassAvg = 0.0;
        qint64 m_lastBeatMs = 0;
};
