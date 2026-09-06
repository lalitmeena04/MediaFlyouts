#pragma once

#include <QObject>

// Track platform subsystem availability.
// UI hides features that need unavailable APIs.
// Flags start available and clear only after reported failure.
class Capabilities : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool audioAvailable READ audioAvailable NOTIFY changed)
        Q_PROPERTY(bool mediaSessionAvailable READ mediaSessionAvailable NOTIFY changed)
        Q_PROPERTY(bool lockKeysAvailable READ lockKeysAvailable NOTIFY changed)
        Q_PROPERTY(bool mediaKeysAvailable READ mediaKeysAvailable NOTIFY changed)

    public:
        explicit Capabilities(QObject* parent = nullptr);

        bool audioAvailable() const;
        bool mediaSessionAvailable() const;
        bool lockKeysAvailable() const;
        bool mediaKeysAvailable() const;

    public slots:
        void setAudioAvailable(bool available);
        void setMediaSessionAvailable(bool available);
        void setLockKeysAvailable(bool available);
        void setMediaKeysAvailable(bool available);

    signals:
        void changed();

    private:
        bool m_audioAvailable = true;
        bool m_mediaSessionAvailable = true;
        bool m_lockKeysAvailable = true;
        bool m_mediaKeysAvailable = true;
};
