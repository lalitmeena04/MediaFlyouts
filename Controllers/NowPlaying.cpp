#include "NowPlaying.h"

#include <QDateTime>

#include "AlbumArtProvider.h"

NowPlaying::NowPlaying(QObject* parent) : QObject(parent) {
    m_sinceUpdate.start();

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

QColor NowPlaying::artAccent() const {
    return m_artAccent;
}

QColor NowPlaying::computeArtAccent(const QImage& art) {
    if(art.isNull())
        return QColor("#4CC2FF");
    const QImage small =
        art.scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
    double r = 0, g = 0, b = 0, weight = 0;
    for(int y = 0; y < small.height(); ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(small.constScanLine(y));
        for(int x = 0; x < small.width(); ++x) {
            const QColor c(line[x]);
            const double s = c.saturationF();
            const double w = 0.1 + s * s;
            r += c.redF() * w;
            g += c.greenF() * w;
            b += c.blueF() * w;
            weight += w;
        }
    }
    if(weight <= 0)
        return QColor("#4CC2FF");
    QColor c = QColor::fromRgbF(r / weight, g / weight, b / weight);
    float h, s, l, a;
    c.getHslF(&h, &s, &l, &a);
    s = qBound(0.45f, s < 0.35f ? 0.5f : s, 1.0f);
    l = qBound(0.5f, l, 0.68f);
    c.setHslF(h < 0 ? 0.55f : h, s, l);
    return c;
}

QString NowPlaying::playerName() const {
    QString id = m_track.sourceApp;
    // AUMID examples: "Family!AppId" or "Spotify.exe".
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
    updateResumeAll();
}

bool NowPlaying::allPaused() const {
    if(!hasMedia() || isPlaying())
        return false;
    return !m_controller || m_controller->playingSessionCount() == 0;
}

bool NowPlaying::canResumeAll() const {
    return m_canResumeAll;
}

void NowPlaying::updateResumeAll() {
    const bool now = !m_pausedByAll.isEmpty() || allPaused();
    if(now == m_canResumeAll)
        return;
    m_canResumeAll = now;
    emit resumeAllChanged();
}

void NowPlaying::pauseAll() {
    if(!m_controller)
        return;
    m_pausedByAll = m_controller->pauseAllSessions();
    updateResumeAll();
}

void NowPlaying::playAll() {
    if(m_controller && (!m_pausedByAll.isEmpty() || allPaused()))
        m_controller->resumeAllSessions();
    m_pausedByAll.clear();
    updateResumeAll();
}

void NowPlaying::applyTrack(const mf::TrackInfo& track) {
    // Timeline updates must not reset the auto-hide timer.
    const bool identityChanged =
        track.title != m_track.title || track.artist != m_track.artist || track.album != m_track.album;
    const bool stateChanged = track.state != m_track.state;

    m_track = track;
    m_sinceUpdate.restart();
    emit changed();

    if(stateChanged && track.state == mf::PlaybackState::Playing)
        m_pausedByAll.clear();
    updateResumeAll();

    // Do not show flyout for initial media state sync at launch.
    if(!m_seenFirstUpdate) {
        m_seenFirstUpdate = true;
        return;
    }

    if(identityChanged) {
        const bool stopped = track.title.isEmpty() && track.artist.isEmpty();
        // Title change inside the manual window means the user pressed next/prev.
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
    m_artAccent = computeArtAccent(art);
    ++m_artRevision;
    emit changed();
}
