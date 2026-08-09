#pragma once

#include <QString>
#include <QtGlobal>

namespace mf {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

struct TrackInfo {
    QString title;
    QString artist;
    QString album;
    PlaybackState state = PlaybackState::Stopped;
    qint64 positionMs = 0;
    qint64 durationMs = 0;
};

// Formats milliseconds as m:ss, or h:mm:ss when at least one hour.
QString formatDuration(qint64 ms);

// Joins title and artist as "Title — Artist", tolerating either being empty.
QString formatTrackLine(const TrackInfo& track);

// Returns playback progress in the closed interval [0, 1].
double playbackProgress(const TrackInfo& track);

} // namespace mf
