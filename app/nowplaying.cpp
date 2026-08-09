#include "nowplaying.h"

#include "albumartprovider.h"

NowPlaying::NowPlaying(QObject* parent)
    : QObject(parent)
{
    m_sinceUpdate.start();

    // repaint interval so the interpolated position advances smoothly between provider updates
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        if (m_track.state == mf::PlaybackState::Playing)
            emit changed();
    });
    m_timer.start();
}

void NowPlaying::setController(mf::IPlaybackController* controller)
{
    m_controller = controller;
}

void NowPlaying::setArtProvider(AlbumArtProvider* provider)
{
    m_artProvider = provider;
}

qint64 NowPlaying::currentPositionMs() const
{
    qint64 pos = m_track.positionMs;
    if (m_track.state == mf::PlaybackState::Playing)
        pos += m_sinceUpdate.elapsed();
    if (pos < 0)
        pos = 0;
    if (m_track.durationMs > 0 && pos > m_track.durationMs)
        pos = m_track.durationMs;
    return pos;
}

QString NowPlaying::title() const { return m_track.title; }

QString NowPlaying::artist() const { return m_track.artist; }

QString NowPlaying::elapsed() const { return mf::formatDuration(currentPositionMs()); }

QString NowPlaying::duration() const { return mf::formatDuration(m_track.durationMs); }

double NowPlaying::progress() const
{
    mf::TrackInfo interpolated = m_track;
    interpolated.positionMs = currentPositionMs();
    return mf::playbackProgress(interpolated);
}

bool NowPlaying::isPlaying() const { return m_track.state == mf::PlaybackState::Playing; }

bool NowPlaying::hasMedia() const
{
    return !m_track.title.isEmpty() || !m_track.artist.isEmpty();
}

QString NowPlaying::artSource() const
{
    if (!m_hasArt)
        return QString();
    return QStringLiteral("image://albumart/cover?rev=%1").arg(m_artRevision);
}

bool NowPlaying::hasArt() const { return m_hasArt; }

void NowPlaying::playPause()
{
    if (m_controller)
        mf::togglePlayPause(*m_controller, m_track.state);
}

void NowPlaying::next()
{
    if (m_controller)
        m_controller->next();
}

void NowPlaying::previous()
{
    if (m_controller)
        m_controller->previous();
}

void NowPlaying::applyTrack(const mf::TrackInfo& track)
{
    m_track = track;
    m_sinceUpdate.restart();
    emit changed();
    emit trackUpdated();
}

void NowPlaying::applyArt(const QImage& art)
{
    if (m_artProvider)
        m_artProvider->setImage(art);
    m_hasArt = !art.isNull();
    ++m_artRevision;
    emit changed();
}
