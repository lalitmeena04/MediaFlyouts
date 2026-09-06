#pragma once

#include <QHash>
#include <QObject>
#include <QVariantList>

#include <vector>

struct ISimpleAudioVolume;

// Enumerate per-app audio sessions on default render device.
// Expose sessions to QML for volume and mute control.
class VolumeMixer : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)

    public:
        explicit VolumeMixer(QObject* parent = nullptr);
        ~VolumeMixer() override;

        QVariantList sessions() const;

    public slots:
        // Rebuild session list from current audio sessions.
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
        // Exe path maps to icon data URI. Avoid extraction every refresh.
        QHash<QString, QString> m_iconCache;
};
