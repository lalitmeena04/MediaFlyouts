#pragma once

#include <atomic>
#include <mutex>
#include <utility>
#include <vector>

#include <QImage>
#include <QObject>

#include <winrt/Windows.Media.Control.h>

#include "PlaybackController.h"
#include "TrackInfo.h"

class WinMediaProvider : public QObject, public mf::IPlaybackController {
        Q_OBJECT

    public:
        explicit WinMediaProvider(QObject* parent = nullptr);
        ~WinMediaProvider() override;

        void start();

        bool available() const;

        void play() override;
        void pause() override;
        void next() override;
        void previous() override;
        void seek(qint64 positionMs) override;
        void setRepeatMode(int mode) override;
        void setShuffle(bool on) override;
        int sessionCount() const override;
        int selectedSessionIndex() const override;
        void cycleSession(int delta) override;
        QStringList pauseAllSessions() override;
        int playingSessionCount() const override;
        void resumeAllSessions() override;

    signals:
        void trackChanged(const mf::TrackInfo& track);
        void artChanged(const QImage& art);
        void sessionsChanged();
        void availabilityChanged();

    private:
        using SessionManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager;
        using Session = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession;

        winrt::fire_and_forget initAsync();
        void hookSession(Session session);
        void publishNoSession();
        void queueRefresh(Session session);
        winrt::fire_and_forget refresh(Session session, int attempt = 0);
        Session currentSession();
        void refreshSessionMeta();
        void watchAllSessions();
        void recountPlaying();

        SessionManager m_manager{nullptr};
        Session m_session{nullptr};
        winrt::event_token m_currentChangedToken{};
        winrt::event_token m_sessionsChangedToken{};
        winrt::event_token m_mediaPropsToken{};
        winrt::event_token m_playbackToken{};
        winrt::event_token m_timelineToken{};
        std::mutex m_mutex;
        std::atomic<int> m_sessionCount{1};
        std::atomic<int> m_selectedIndex{0};
        std::atomic<int> m_playingCount{0};
        std::mutex m_watchMutex;
        std::vector<std::pair<Session, winrt::event_token>> m_watched;
        bool m_manual = false;
        std::mutex m_lastMutex;
        mf::TrackInfo m_lastTrack;
        QImage m_lastArt;
};
