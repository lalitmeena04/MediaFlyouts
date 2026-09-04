#pragma once

#include <QObject>

struct IMMDeviceEnumerator;
struct IAudioEndpointVolume;
struct VolumeCallback;
struct DeviceNotificationClient;

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
        // true once a default render endpoint has been acquired (Core Audio available)
        bool available() const;

        Q_INVOKABLE void setVolume(int percent);
        Q_INVOKABLE void toggleMute();

        // re-reads the endpoint and always signals, so the flyout shows even at min/max
        void handleVolumeKey();

        // re-bind to whatever the current default render device is (after a device switch)
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
