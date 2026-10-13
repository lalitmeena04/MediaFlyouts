#pragma once

#include <QHash>
#include <QObject>
#include <QVariantList>

#include <vector>

struct ISimpleAudioVolume;

class VolumeMixer : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)

    public:
        explicit VolumeMixer(QObject* parent = nullptr);
        ~VolumeMixer() override;

        QVariantList sessions() const;

    public slots:
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
        QHash<QString, QString> m_iconCache;
};
