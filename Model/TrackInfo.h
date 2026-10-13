#pragma once

#include <QString>
#include <QtGlobal>

namespace mf {

enum class PlaybackState { Stopped, Playing, Paused };

struct TrackInfo {
        QString title;
        QString artist;
        QString album;
        QString sourceApp;
        PlaybackState state = PlaybackState::Stopped;
        qint64 positionMs = 0;
        qint64 durationMs = 0;
        int repeatMode = 0; // 0 none, 1 track, 2 list
        bool shuffle = false;
        bool canSeek = false;
        bool canRepeat = false;
        bool canShuffle = false;
};

QString formatDuration(qint64 ms);

QString formatTrackLine(const TrackInfo& track);

double playbackProgress(const TrackInfo& track);

} // namespace mf
