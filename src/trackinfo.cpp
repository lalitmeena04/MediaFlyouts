#include "trackinfo.h"

namespace mf {

QString formatDuration(qint64 ms)
{
    if (ms < 0)
        ms = 0;

    const qint64 totalSeconds = ms / 1000;
    const qint64 seconds = totalSeconds % 60;
    const qint64 minutes = (totalSeconds / 60) % 60;
    const qint64 hours = totalSeconds / 3600;

    if (hours > 0)
        return QStringLiteral("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));

    return QStringLiteral("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

QString formatTrackLine(const TrackInfo& track)
{
    if (track.artist.isEmpty())
        return track.title;
    if (track.title.isEmpty())
        return track.artist;
    return track.title + QStringLiteral(" — ") + track.artist;
}

double playbackProgress(const TrackInfo& track)
{
    if (track.durationMs <= 0)
        return 0.0;

    double progress = static_cast<double>(track.positionMs) / static_cast<double>(track.durationMs);
    if (progress < 0.0)
        progress = 0.0;
    if (progress > 1.0)
        progress = 1.0;
    return progress;
}

} // namespace mf
