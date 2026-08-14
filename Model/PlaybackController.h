#pragma once

#include "TrackInfo.h"

namespace mf {

// Abstraction over a media transport so playback logic stays testable and mockable.
class IPlaybackController {
    public:
        virtual ~IPlaybackController() = default;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void next() = 0;
        virtual void previous() = 0;
        // optional transport extras; no-ops unless a provider supports them
        virtual void seek(qint64 positionMs) {
            (void)positionMs;
        }
        virtual void setRepeatMode(int mode) {
            (void)mode;
        }
        virtual void setShuffle(bool on) {
            (void)on;
        }
};

// Pauses when currently playing, otherwise resumes playback.
void togglePlayPause(IPlaybackController& controller, PlaybackState state);

} // namespace mf
