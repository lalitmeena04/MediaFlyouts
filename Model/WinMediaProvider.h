#pragma once

#include <QImage>
#include <QObject>
#include <mutex>

#include <winrt/Windows.Media.Control.h>

#include "PlaybackController.h"
#include "TrackInfo.h"

// Bridges Windows System Media Transport Controls (SMTC) to Qt, and drives the active session.
class WinMediaProvider : public QObject, public mf::IPlaybackController {
        Q_OBJECT

    public:
        explicit WinMediaProvider(QObject* parent = nullptr);
        ~WinMediaProvider() override;

        void start();

        // mf::IPlaybackController — forwarded to the currently active session.
        void play() override;
        void pause() override;
        void next() override;
        void previous() override;
        void seek(qint64 positionMs) override;
        void setRepeatMode(int mode) override;
        void setShuffle(bool on) override;

    signals:
        void trackChanged(const mf::TrackInfo& track);
        void artChanged(const QImage& art);

    private:
        using SessionManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager;
        using Session = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession;

        winrt::fire_and_forget initAsync();
        void hookSession(Session session);
        winrt::fire_and_forget refresh(Session session);
        Session currentSession();

        SessionManager m_manager{nullptr};
        Session m_session{nullptr};
        winrt::event_token m_currentChangedToken{};
        winrt::event_token m_mediaPropsToken{};
        winrt::event_token m_playbackToken{};
        winrt::event_token m_timelineToken{};
        std::mutex m_mutex;
};
