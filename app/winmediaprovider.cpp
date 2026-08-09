#include "winmediaprovider.h"

#include <chrono>

#include <QMetaObject>
#include <QString>

#include <winrt/Windows.Foundation.h>

using namespace winrt::Windows::Media::Control;
using PlaybackStatus = GlobalSystemMediaTransportControlsSessionPlaybackStatus;

WinMediaProvider::WinMediaProvider(QObject* parent)
    : QObject(parent)
{
}

WinMediaProvider::~WinMediaProvider()
{
    try {
        if (m_manager && m_currentChangedToken.value)
            m_manager.CurrentSessionChanged(m_currentChangedToken);
    } catch (...) {
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_session) {
        try {
            if (m_mediaPropsToken.value)
                m_session.MediaPropertiesChanged(m_mediaPropsToken);
            if (m_playbackToken.value)
                m_session.PlaybackInfoChanged(m_playbackToken);
            if (m_timelineToken.value)
                m_session.TimelinePropertiesChanged(m_timelineToken);
        } catch (...) {
        }
    }
}

void WinMediaProvider::start()
{
    initAsync();
}

winrt::fire_and_forget WinMediaProvider::initAsync()
{
    try {
        m_manager = co_await SessionManager::RequestAsync();
    } catch (...) {
        co_return;
    }

    m_currentChangedToken = m_manager.CurrentSessionChanged(
        [this](auto&&, auto&&) { hookSession(m_manager.GetCurrentSession()); });

    hookSession(m_manager.GetCurrentSession());
}

void WinMediaProvider::hookSession(Session session)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_session) {
            try {
                if (m_mediaPropsToken.value)
                    m_session.MediaPropertiesChanged(m_mediaPropsToken);
                if (m_playbackToken.value)
                    m_session.PlaybackInfoChanged(m_playbackToken);
                if (m_timelineToken.value)
                    m_session.TimelinePropertiesChanged(m_timelineToken);
            } catch (...) {
            }
            m_mediaPropsToken = {};
            m_playbackToken = {};
            m_timelineToken = {};
        }
        m_session = session;
    }

    if (!session) {
        const mf::TrackInfo empty;
        QMetaObject::invokeMethod(this, [this, empty] { emit trackChanged(empty); }, Qt::QueuedConnection);
        return;
    }

    m_mediaPropsToken = session.MediaPropertiesChanged([this](Session s, auto&&) { refresh(s); });
    m_playbackToken = session.PlaybackInfoChanged([this](Session s, auto&&) { refresh(s); });
    m_timelineToken = session.TimelinePropertiesChanged([this](Session s, auto&&) { refresh(s); });

    refresh(session);
}

// runs as a coroutine: awaiting (never .get()) keeps it safe to resume on the STA thread
winrt::fire_and_forget WinMediaProvider::refresh(Session session)
{
    if (!session)
        co_return;

    mf::TrackInfo track;

    try {
        auto props = co_await session.TryGetMediaPropertiesAsync();
        track.title = QString::fromWCharArray(props.Title().c_str());
        track.artist = QString::fromWCharArray(props.Artist().c_str());
        track.album = QString::fromWCharArray(props.AlbumTitle().c_str());
    } catch (...) {
    }

    try {
        switch (session.GetPlaybackInfo().PlaybackStatus()) {
        case PlaybackStatus::Playing:
            track.state = mf::PlaybackState::Playing;
            break;
        case PlaybackStatus::Paused:
            track.state = mf::PlaybackState::Paused;
            break;
        default:
            track.state = mf::PlaybackState::Stopped;
            break;
        }
    } catch (...) {
    }

    try {
        auto timeline = session.GetTimelineProperties();
        track.durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                               timeline.EndTime() - timeline.StartTime())
                               .count();
        track.positionMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                               timeline.Position() - timeline.StartTime())
                               .count();
    } catch (...) {
    }

    QMetaObject::invokeMethod(this, [this, track] { emit trackChanged(track); }, Qt::QueuedConnection);
}

WinMediaProvider::Session WinMediaProvider::currentSession()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_session;
}

void WinMediaProvider::play()
{
    if (auto s = currentSession())
        s.TryPlayAsync();
}

void WinMediaProvider::pause()
{
    if (auto s = currentSession())
        s.TryPauseAsync();
}

void WinMediaProvider::next()
{
    if (auto s = currentSession())
        s.TrySkipNextAsync();
}

void WinMediaProvider::previous()
{
    if (auto s = currentSession())
        s.TrySkipPreviousAsync();
}
