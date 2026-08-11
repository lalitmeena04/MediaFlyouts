#pragma once

#include <QObject>

struct IMMDeviceEnumerator;
struct IAudioEndpointVolume;
struct VolumeCallback;

// Bridges the default render endpoint's master volume (Core Audio) to Qt.
class VolumeController : public QObject {
        Q_OBJECT
        Q_PROPERTY(int volume READ volume NOTIFY changed)
        Q_PROPERTY(bool muted READ muted NOTIFY changed)

    public:
        explicit VolumeController(QObject* parent = nullptr);
        ~VolumeController() override;

        void start();

        int volume() const;
        bool muted() const;

        Q_INVOKABLE void setVolume(int percent);
        Q_INVOKABLE void toggleMute();

        // re-reads the endpoint and always signals, so the flyout shows even at min/max
        void handleVolumeKey();

    signals:
        void changed();
        void volumeChangedExternally();

    private:
        friend struct VolumeCallback;
        void applyState(int percent, bool muted, bool external);

        IMMDeviceEnumerator* m_enumerator = nullptr;
        IAudioEndpointVolume* m_endpoint = nullptr;
        VolumeCallback* m_callback = nullptr;
        int m_volume = 0;
        bool m_muted = false;
};
