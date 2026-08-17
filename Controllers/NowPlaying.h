#pragma once

#include <QElapsedTimer>
#include <QImage>
#include <QObject>
#include <QString>
#include <QTimer>

#include "PlaybackController.h"
#include "TrackInfo.h"

class AlbumArtProvider;

// QML-facing now-playing model; fed by a media provider and forwards transport commands to a controller.
class NowPlaying : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString title READ title NOTIFY changed)
        Q_PROPERTY(QString artist READ artist NOTIFY changed)
        Q_PROPERTY(QString elapsed READ elapsed NOTIFY changed)
        Q_PROPERTY(QString duration READ duration NOTIFY changed)
        Q_PROPERTY(double progress READ progress NOTIFY changed)
        Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY changed)
        Q_PROPERTY(bool hasMedia READ hasMedia NOTIFY changed)
        Q_PROPERTY(QString artSource READ artSource NOTIFY changed)
        Q_PROPERTY(bool hasArt READ hasArt NOTIFY changed)
        Q_PROPERTY(QString playerName READ playerName NOTIFY changed)
        Q_PROPERTY(int repeatMode READ repeatMode NOTIFY changed)
        Q_PROPERTY(bool shuffle READ shuffle NOTIFY changed)
        Q_PROPERTY(bool canSeek READ canSeek NOTIFY changed)
        Q_PROPERTY(bool canRepeat READ canRepeat NOTIFY changed)
        Q_PROPERTY(bool canShuffle READ canShuffle NOTIFY changed)
        Q_PROPERTY(int sessionCount READ sessionCount NOTIFY sessionsChanged)
        Q_PROPERTY(int selectedSessionIndex READ selectedSessionIndex NOTIFY sessionsChanged)

    public:
        explicit NowPlaying(QObject* parent = nullptr);

        void setController(mf::IPlaybackController* controller);
        void setArtProvider(AlbumArtProvider* provider);

        QString title() const;
        QString artist() const;
        QString elapsed() const;
        QString duration() const;
        double progress() const;
        bool isPlaying() const;
        bool hasMedia() const;
        QString artSource() const;
        bool hasArt() const;
        QString playerName() const;
        int repeatMode() const;
        bool shuffle() const;
        bool canSeek() const;
        bool canRepeat() const;
        bool canShuffle() const;
        int sessionCount() const;
        int selectedSessionIndex() const;

        Q_INVOKABLE void playPause();
        Q_INVOKABLE void next();
        Q_INVOKABLE void previous();
        Q_INVOKABLE void seekToFraction(double fraction);
        Q_INVOKABLE void cycleRepeat();
        Q_INVOKABLE void toggleShuffle();
        // cycle the displayed media source (multi-app switcher on the widget)
        Q_INVOKABLE void cycleSession(int delta);

    public slots:
        void applyTrack(const mf::TrackInfo& track);
        void applyArt(const QImage& art);
        void noteMediaKey();
        // provider reports the set of media sessions changed
        void noteSessionsChanged();
        // ms before the current track ends that the up-next flyout should appear
        void setNextUpLeadMs(int ms);

    signals:
        void changed();
        void trackUpdated();
        void nextUpRequested();
        void sessionsChanged();

    private:
        qint64 currentPositionMs() const;

        mf::TrackInfo m_track;
        QElapsedTimer m_sinceUpdate;
        QTimer m_timer;
        mf::IPlaybackController* m_controller = nullptr;
        AlbumArtProvider* m_artProvider = nullptr;
        int m_artRevision = 0;
        bool m_hasArt = false;
        qint64 m_lastMediaKeyMs = 0;
        bool m_seenFirstUpdate = false;
        // up-next preview state
        int m_nextUpLeadMs = 3000;
        bool m_nextUpFired = false;
};
