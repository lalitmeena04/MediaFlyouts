#pragma once

#include <QObject>

struct IMMDeviceEnumerator;
struct IAudioEndpointVolume;
struct VolumeCallback;
struct DeviceNotificationClient;

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
        bool available() const;

        Q_INVOKABLE void setVolume(int percent);
        Q_INVOKABLE void toggleMute();

        void handleVolumeKey();

        void rebindDefaultDevice();

    signals:
        void changed();
        void volumeChangedExternally();
        void availabilityChanged();

    private:
        friend struct VolumeCallback;
        friend struct DeviceNotificationClient;
        void applyState(int percent, bool muted, bool external);
        void releaseEndpoint();
        bool acquireEndpoint();

        IMMDeviceEnumerator* m_enumerator = nullptr;
        IAudioEndpointVolume* m_endpoint = nullptr;
        VolumeCallback* m_callback = nullptr;
        DeviceNotificationClient* m_deviceNotifier = nullptr;
        int m_volume = 0;
        bool m_muted = false;
};
