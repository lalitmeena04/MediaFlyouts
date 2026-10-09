#include "WinMediaProvider.h"

#include <chrono>

#include <QByteArray>
#include <QImage>
#include <QMetaObject>
#include <QString>
#include <QStringList>

#include "Logging.h"
#include "SessionSelection.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Media::Control;
using PlaybackStatus = GlobalSystemMediaTransportControlsSessionPlaybackStatus;
namespace streams = winrt::Windows::Storage::Streams;

WinMediaProvider::WinMediaProvider(QObject* parent) : QObject(parent) {}

WinMediaProvider::~WinMediaProvider() {
    try {
        if(m_manager && m_currentChangedToken.value)
            m_manager.CurrentSessionChanged(m_currentChangedToken);
        if(m_manager && m_sessionsChangedToken.value)
            m_manager.SessionsChanged(m_sessionsChangedToken);
    } catch(...) {
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_session) {
        try {
            if(m_mediaPropsToken.value)
                m_session.MediaPropertiesChanged(m_mediaPropsToken);
            if(m_playbackToken.value)
                m_session.PlaybackInfoChanged(m_playbackToken);
            if(m_timelineToken.value)
                m_session.TimelinePropertiesChanged(m_timelineToken);
        } catch(...) {
        }
    }

    std::lock_guard<std::mutex> watchLock(m_watchMutex);
    for(auto& [session, token] : m_watched) {
        try {
            session.PlaybackInfoChanged(token);
        } catch(...) {
        }
    }
}

void WinMediaProvider::start() {
    initAsync();
}

bool WinMediaProvider::available() const {
    return m_manager != nullptr;
}

winrt::fire_and_forget WinMediaProvider::initAsync() {
    try {
        m_manager = co_await SessionManager::RequestAsync();
    } catch(const winrt::hresult_error& e) {
        TRC_WARN("media: SMTC session manager unavailable (hr={:#x})", static_cast<unsigned long>(e.code()));
        QMetaObject::invokeMethod(this, [this] { emit availabilityChanged(); }, Qt::QueuedConnection);
        co_return;
    } catch(...) {
        TRC_WARN("media: SMTC session manager request threw");
        QMetaObject::invokeMethod(this, [this] { emit availabilityChanged(); }, Qt::QueuedConnection);
        co_return;
    }
    TRC_INFO("media: SMTC session manager ready");
    QMetaObject::invokeMethod(this, [this] { emit availabilityChanged(); }, Qt::QueuedConnection);

    m_currentChangedToken = m_manager.CurrentSessionChanged([this](auto&&, auto&&) {
        // OS current session controls. User selection overrides it.
        if(!m_manual) {
            hookSession(m_manager.GetCurrentSession());
            refreshSessionMeta();
            QMetaObject::invokeMethod(this, [this] { emit sessionsChanged(); }, Qt::QueuedConnection);
        }
    });

    m_sessionsChangedToken = m_manager.SessionsChanged([this](auto&&, auto&&) {
        // If chosen session is gone, use OS current session.
        if(m_manual) {
            auto sessions = m_manager.GetSessions();
            QString selectedAppId;
            if(const Session sel = currentSession()) {
                try {
                    selectedAppId = QString::fromWCharArray(sel.SourceAppUserModelId().c_str());
                } catch(...) {
                }
            }
            bool stillThere = false;
            if(!selectedAppId.isEmpty()) {
                for(uint32_t i = 0; i < sessions.Size(); ++i) {
                    try {
                        if(QString::fromWCharArray(sessions.GetAt(i).SourceAppUserModelId().c_str()) == selectedAppId) {
                            stillThere = true;
                            break;
                        }
                    } catch(...) {
                    }
                }
            }
            if(!stillThere) {
                m_manual = false;
                hookSession(m_manager.GetCurrentSession());
            }
        }
        watchAllSessions();
        refreshSessionMeta();
        QMetaObject::invokeMethod(this, [this] { emit sessionsChanged(); }, Qt::QueuedConnection);
    });

    hookSession(m_manager.GetCurrentSession());
    watchAllSessions();
    refreshSessionMeta();
    QMetaObject::invokeMethod(this, [this] { emit sessionsChanged(); }, Qt::QueuedConnection);
}

void WinMediaProvider::watchAllSessions() {
    if(!m_manager)
        return;
    std::lock_guard<std::mutex> lock(m_watchMutex);
    for(auto& [session, token] : m_watched) {
        try {
            session.PlaybackInfoChanged(token);
        } catch(...) {
        }
    }
    m_watched.clear();
    try {
        auto sessions = m_manager.GetSessions();
        for(uint32_t i = 0; i < sessions.Size(); ++i) {
            try {
                Session s = sessions.GetAt(i);
                auto token = s.PlaybackInfoChanged([this](auto&&, auto&&) { recountPlaying(); });
                m_watched.emplace_back(std::move(s), token);
            } catch(...) {
            }
        }
    } catch(...) {
    }
    recountPlaying();
}

void WinMediaProvider::recountPlaying() {
    if(!m_manager)
        return;
    int playing = 0;
    try {
        auto sessions = m_manager.GetSessions();
        for(uint32_t i = 0; i < sessions.Size(); ++i) {
            try {
                if(sessions.GetAt(i).GetPlaybackInfo().PlaybackStatus() == PlaybackStatus::Playing)
                    ++playing;
            } catch(...) {
            }
        }
    } catch(...) {
    }
    if(m_playingCount.exchange(playing) != playing)
        QMetaObject::invokeMethod(this, [this] { emit sessionsChanged(); }, Qt::QueuedConnection);
}

int WinMediaProvider::playingSessionCount() const {
    return m_playingCount.load();
}

void WinMediaProvider::resumeAllSessions() {
    if(!m_manager)
        return;
    try {
        auto sessions = m_manager.GetSessions();
        for(uint32_t i = 0; i < sessions.Size(); ++i) {
            try {
                auto s = sessions.GetAt(i);
                // Stopped or closed sessions stay as they are.
                if(s.GetPlaybackInfo().PlaybackStatus() == PlaybackStatus::Paused)
                    s.TryPlayAsync();
            } catch(...) {
            }
        }
    } catch(...) {
    }
}

void WinMediaProvider::refreshSessionMeta() {
    if(!m_manager)
        return;
    auto sessions = m_manager.GetSessions();
    const int n = static_cast<int>(sessions.Size());
    m_sessionCount = mf::normalizedSessionCount(n);

    // Match by app id, not WinRT identity. Separate calls can return different proxy objects.
    QString selectedAppId;
    if(const Session sel = currentSession()) {
        try {
            selectedAppId = QString::fromWCharArray(sel.SourceAppUserModelId().c_str());
        } catch(...) {
        }
    }

    QStringList appIds;
    appIds.reserve(n);
    for(int i = 0; i < n; ++i) {
        try {
            appIds.append(QString::fromWCharArray(sessions.GetAt(static_cast<uint32_t>(i)).SourceAppUserModelId().c_str()));
        } catch(...) {
            appIds.append(QString());
        }
    }
    m_selectedIndex = mf::selectedSessionIndexByAppId(appIds, selectedAppId);
}

int WinMediaProvider::sessionCount() const {
    return m_sessionCount.load();
}

int WinMediaProvider::selectedSessionIndex() const {
    return m_selectedIndex.load();
}

void WinMediaProvider::cycleSession(int delta) {
    if(!m_manager || delta == 0)
        return;
    auto sessions = m_manager.GetSessions();
    const int n = static_cast<int>(sessions.Size());
    if(n <= 1)
        return;

    QString selectedAppId;
    if(const Session sel = currentSession()) {
        try {
            selectedAppId = QString::fromWCharArray(sel.SourceAppUserModelId().c_str());
        } catch(...) {
        }
    }
    QStringList appIds;
    appIds.reserve(n);
    for(int i = 0; i < n; ++i) {
        try {
            appIds.append(QString::fromWCharArray(sessions.GetAt(static_cast<uint32_t>(i)).SourceAppUserModelId().c_str()));
        } catch(...) {
            appIds.append(QString());
        }
    }
    const int current = mf::selectedSessionIndexByAppId(appIds, selectedAppId);
    const int next = ((current + delta) % n + n) % n;
    m_manual = true;
    hookSession(sessions.GetAt(static_cast<uint32_t>(next)));
    refreshSessionMeta();
    QMetaObject::invokeMethod(this, [this] { emit sessionsChanged(); }, Qt::QueuedConnection);
}

QStringList WinMediaProvider::pauseAllSessions() {
    QStringList paused;
    if(!m_manager)
        return paused;
    try {
        auto sessions = m_manager.GetSessions();
        for(uint32_t i = 0; i < sessions.Size(); ++i) {
            try {
                auto s = sessions.GetAt(i);
                if(s.GetPlaybackInfo().PlaybackStatus() != PlaybackStatus::Playing)
                    continue;
                paused.append(QString::fromWCharArray(s.SourceAppUserModelId().c_str()));
                s.TryPauseAsync();
            } catch(...) {
            }
        }
    } catch(...) {
    }
    return paused;
}

void WinMediaProvider::hookSession(Session session) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_session) {
            try {
                if(m_mediaPropsToken.value)
                    m_session.MediaPropertiesChanged(m_mediaPropsToken);
                if(m_playbackToken.value)
                    m_session.PlaybackInfoChanged(m_playbackToken);
                if(m_timelineToken.value)
                    m_session.TimelinePropertiesChanged(m_timelineToken);
            } catch(...) {
            }
            m_mediaPropsToken = {};
            m_playbackToken = {};
            m_timelineToken = {};
        }
        m_session = session;
    }

    if(!session) {
        const mf::TrackInfo empty;
        QMetaObject::invokeMethod(this, [this, empty] { emit trackChanged(empty); }, Qt::QueuedConnection);
        return;
    }

    m_mediaPropsToken = session.MediaPropertiesChanged([this](Session s, auto&&) { queueRefresh(s); });
    m_playbackToken = session.PlaybackInfoChanged([this](Session s, auto&&) { queueRefresh(s); });
    m_timelineToken = session.TimelinePropertiesChanged([this](Session s, auto&&) { queueRefresh(s); });

    refresh(session);
}

void WinMediaProvider::queueRefresh(Session session) {
    QMetaObject::invokeMethod(this, [this, session] { refresh(session); }, Qt::QueuedConnection);
}

// Coroutine. Use co_await. Safe for STA thread.
winrt::fire_and_forget WinMediaProvider::refresh(Session session, int attempt) {
    if(!session)
        co_return;

    mf::TrackInfo track;

    GlobalSystemMediaTransportControlsSessionMediaProperties props{nullptr};
    QString subtitle;
    try {
        props = co_await session.TryGetMediaPropertiesAsync();
        if(props) {
            track.title = QString::fromWCharArray(props.Title().c_str());
            track.artist = QString::fromWCharArray(props.Artist().c_str());
            track.album = QString::fromWCharArray(props.AlbumTitle().c_str());
            subtitle = QString::fromWCharArray(props.Subtitle().c_str());
        }
    } catch(...) {
    }

    // Video apps often leave Artist empty. Use Subtitle as second line.
    if(track.artist.isEmpty())
        track.artist = subtitle;

    try {
        auto info = session.GetPlaybackInfo();
        switch(info.PlaybackStatus()) {
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

        auto controls = info.Controls();
        track.canSeek = controls.IsPlaybackPositionEnabled();
        track.canRepeat = controls.IsRepeatEnabled();
        track.canShuffle = controls.IsShuffleEnabled();
        if(auto repeat = info.AutoRepeatMode())
            track.repeatMode = static_cast<int>(repeat.Value());
        if(auto shuffle = info.IsShuffleActive())
            track.shuffle = shuffle.Value();
    } catch(...) {
    }

    try {
        track.sourceApp = QString::fromWCharArray(session.SourceAppUserModelId().c_str());
    } catch(...) {
    }

    try {
        auto timeline = session.GetTimelineProperties();
        track.durationMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(timeline.EndTime() - timeline.StartTime()).count();
        track.positionMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(timeline.Position() - timeline.StartTime()).count();
    } catch(...) {
    }

    QImage art;
    if(props) {
        try {
            auto thumbRef = props.Thumbnail();
            if(thumbRef) {
                auto stream = co_await thumbRef.OpenReadAsync();
                const uint32_t size = static_cast<uint32_t>(stream.Size());
                if(size > 0) {
                    streams::Buffer buffer(size);
                    auto readBuffer = co_await stream.ReadAsync(buffer, size, streams::InputStreamOptions::None);
                    QByteArray bytes;
                    bytes.resize(static_cast<int>(readBuffer.Length()));
                    auto reader = streams::DataReader::FromBuffer(readBuffer);
                    reader.ReadBytes(
                        winrt::array_view<uint8_t>(reinterpret_cast<uint8_t*>(bytes.data()),
                                                   reinterpret_cast<uint8_t*>(bytes.data()) + bytes.size()));
                    art.loadFromData(bytes);
                }
            }
        } catch(...) {
        }
    }

    const bool freshEmpty = track.title.isEmpty() && track.artist.isEmpty() && track.album.isEmpty();
    const bool active = track.state == mf::PlaybackState::Playing || track.state == mf::PlaybackState::Paused;

    // Streaming apps can report empty metadata during buffering.
    // Keep last good metadata for same source app.
    {
        std::lock_guard<std::mutex> lock(m_lastMutex);
        const bool lastHadMeta =
            !(m_lastTrack.title.isEmpty() && m_lastTrack.artist.isEmpty() && m_lastTrack.album.isEmpty());
        if(freshEmpty && active && lastHadMeta && !track.sourceApp.isEmpty() &&
           track.sourceApp == m_lastTrack.sourceApp) {
            track.title = m_lastTrack.title;
            track.artist = m_lastTrack.artist;
            track.album = m_lastTrack.album;
            if(art.isNull())
                art = m_lastArt;
        }
        m_lastTrack = track;
        if(!art.isNull())
            m_lastArt = art;
        else if(!active)
            m_lastArt = QImage(); // Media stopped. Drop stale artwork.
    }

    QMetaObject::invokeMethod(
        this,
        [this, track, art] {
            emit trackChanged(track);
            emit artChanged(art);
        },
        Qt::QueuedConnection);

    // Some apps send metadata after playback starts. Re-poll a few times.
    if(freshEmpty && active && attempt < 3) {
        co_await winrt::resume_after(std::chrono::milliseconds(500));
        QString stillApp;
        if(const Session cur = currentSession()) {
            try {
                stillApp = QString::fromWCharArray(cur.SourceAppUserModelId().c_str());
            } catch(...) {
            }
        }
        if(!track.sourceApp.isEmpty() && stillApp == track.sourceApp)
            refresh(session, attempt + 1);
    }
}

WinMediaProvider::Session WinMediaProvider::currentSession() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_session;
}

void WinMediaProvider::play() {
    if(auto s = currentSession())
        s.TryPlayAsync();
}

void WinMediaProvider::pause() {
    if(auto s = currentSession())
        s.TryPauseAsync();
}

void WinMediaProvider::next() {
    if(auto s = currentSession())
        s.TrySkipNextAsync();
}

void WinMediaProvider::previous() {
    if(auto s = currentSession())
        s.TrySkipPreviousAsync();
}

void WinMediaProvider::seek(qint64 positionMs) {
    if(auto s = currentSession()) {
        try {
            const auto start = s.GetTimelineProperties().StartTime();
            s.TryChangePlaybackPositionAsync(start.count() + positionMs * 10000); // 100ns ticks
        } catch(...) {
        }
    }
}

void WinMediaProvider::setRepeatMode(int mode) {
    if(auto s = currentSession()) {
        using RepeatMode = winrt::Windows::Media::MediaPlaybackAutoRepeatMode;
        const RepeatMode m = mode == 1 ? RepeatMode::Track : mode == 2 ? RepeatMode::List : RepeatMode::None;
        try {
            s.TryChangeAutoRepeatModeAsync(m);
        } catch(...) {
        }
    }
}

void WinMediaProvider::setShuffle(bool on) {
    if(auto s = currentSession()) {
        try {
            s.TryChangeShuffleActiveAsync(on);
        } catch(...) {
        }
    }
}
