#include "NowPlaying.h"

#include <QDateTime>

#include "AlbumArtProvider.h"

NowPlaying::NowPlaying(QObject* parent) : QObject(parent) {
    m_sinceUpdate.start();

    // repaint interval so the interpolated position advances smoothly between provider updates
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        if(m_track.state == mf::PlaybackState::Playing)
            emit changed();
    });
    m_timer.start();
}

void NowPlaying::setController(mf::IPlaybackController* controller) {
    m_controller = controller;
}

void NowPlaying::setArtProvider(AlbumArtProvider* provider) {
    m_artProvider = provider;
}

qint64 NowPlaying::currentPositionMs() const {
    qint64 pos = m_track.positionMs;
    if(m_track.state == mf::PlaybackState::Playing)
        pos += m_sinceUpdate.elapsed();
    if(pos < 0)
        pos = 0;
    if(m_track.durationMs > 0 && pos > m_track.durationMs)
        pos = m_track.durationMs;
    return pos;
}

QString NowPlaying::title() const {
    return m_track.title;
}

QString NowPlaying::artist() const {
    return m_track.artist;
}

QString NowPlaying::elapsed() const {
    return mf::formatDuration(currentPositionMs());
}

QString NowPlaying::duration() const {
    return mf::formatDuration(m_track.durationMs);
}

double NowPlaying::progress() const {
    mf::TrackInfo interpolated = m_track;
    interpolated.positionMs = currentPositionMs();
    return mf::playbackProgress(interpolated);
}

bool NowPlaying::isPlaying() const {
    return m_track.state == mf::PlaybackState::Playing;
}

bool NowPlaying::hasMedia() const {
    return !m_track.title.isEmpty() || !m_track.artist.isEmpty();
}

QString NowPlaying::artSource() const {
    if(!m_hasArt)
        return QString();
    return QStringLiteral("image://albumart/cover?rev=%1").arg(m_artRevision);
}

bool NowPlaying::hasArt() const {
    return m_hasArt;
}

QString NowPlaying::playerName() const {
    QString id = m_track.sourceApp;
    // AUMIDs look like "Family!AppId" for packaged apps, or "Spotify.exe" for desktop apps
    const int bang = id.lastIndexOf(QLatin1Char('!'));
    if(bang >= 0)
        id = id.mid(bang + 1);
    if(id.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive))
        id.chop(4);
    return id;
}

int NowPlaying::repeatMode() const {
    return m_track.repeatMode;
}

bool NowPlaying::shuffle() const {
    return m_track.shuffle;
}

bool NowPlaying::canSeek() const {
    return m_track.canSeek;
}

bool NowPlaying::canRepeat() const {
    return m_track.canRepeat;
}

bool NowPlaying::canShuffle() const {
    return m_track.canShuffle;
}

void NowPlaying::playPause() {
    if(m_controller)
        mf::togglePlayPause(*m_controller, m_track.state);
}

void NowPlaying::next() {
    if(m_controller)
        m_controller->next();
}

void NowPlaying::previous() {
    if(m_controller)
        m_controller->previous();
}

void NowPlaying::seekToFraction(double fraction) {
    if(m_controller && m_track.durationMs > 0) {
        const double clamped = fraction < 0 ? 0 : (fraction > 1 ? 1 : fraction);
        m_controller->seek(static_cast<qint64>(clamped * m_track.durationMs));
    }
}

void NowPlaying::cycleRepeat() {
    if(m_controller)
        m_controller->setRepeatMode((m_track.repeatMode + 1) % 3);
}

void NowPlaying::toggleShuffle() {
    if(m_controller)
        m_controller->setShuffle(!m_track.shuffle);
}

int NowPlaying::sessionCount() const {
    return m_controller ? m_controller->sessionCount() : 1;
}

int NowPlaying::selectedSessionIndex() const {
    return m_controller ? m_controller->selectedSessionIndex() : 0;
}

void NowPlaying::cycleSession(int delta) {
    if(m_controller)
        m_controller->cycleSession(delta);
}

void NowPlaying::noteSessionsChanged() {
    emit sessionsChanged();
}

void NowPlaying::applyTrack(const mf::TrackInfo& track) {
    // only a new track identity or a play/pause change should reveal the flyout;
    // frequent timeline/position updates must not keep resetting the auto-hide timer
    const bool identityChanged =
        track.title != m_track.title || track.artist != m_track.artist || track.album != m_track.album;
    const bool stateChanged = track.state != m_track.state;

    m_track = track;
    m_sinceUpdate.restart();
    emit changed();

    // don't pop any flyout for the initial media state sync at launch
    if(!m_seenFirstUpdate) {
        m_seenFirstUpdate = true;
        return;
    }

    if(identityChanged) {
        const bool stopped = track.title.isEmpty() && track.artist.isEmpty();
        // a title change within the manual window means the user pressed next/prev -> media flyout;
        // otherwise the track auto-advanced -> Next Up flyout (which now shows the newly started track)
        const bool manual = (QDateTime::currentMSecsSinceEpoch() - m_lastMediaKeyMs) < 1500;
        if(stopped || manual)
            emit trackUpdated();
        else
            emit nextUpRequested();
    } else if(stateChanged) {
        emit trackUpdated();
    }
}

void NowPlaying::noteMediaKey() {
    m_lastMediaKeyMs = QDateTime::currentMSecsSinceEpoch();
}

void NowPlaying::applyArt(const QImage& art) {
    if(m_artProvider)
        m_artProvider->setImage(art);
    m_hasArt = !art.isNull();
    ++m_artRevision;
    emit changed();
}
