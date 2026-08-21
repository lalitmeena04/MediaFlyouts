#pragma once

#include <QObject>
#include <QVariantList>

#include <vector>

struct ISimpleAudioVolume;

// Enumerates per-application audio sessions on the default render device and
// exposes them to QML so each app's volume / mute can be controlled.
class VolumeMixer : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)

    public:
        explicit VolumeMixer(QObject* parent = nullptr);
        ~VolumeMixer() override;

        QVariantList sessions() const;

    public slots:
        // rebuild the session list from the current audio sessions
        void refresh();
        void setSessionVolume(int index, int volume);
        void setSessionMute(int index, bool muted);

    signals:
        void sessionsChanged();

    private:
        void clear();

        struct Entry {
                ISimpleAudioVolume* volume = nullptr;
                QString name;
        };
        std::vector<Entry> m_entries;
        QVariantList m_sessions;
};
