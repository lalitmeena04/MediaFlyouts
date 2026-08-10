#pragma once

#include "trackinfo.h"

namespace mf {

// Abstraction over a media transport so playback logic stays testable and mockable.
class IPlaybackController {
    public:
        virtual ~IPlaybackController() = default;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void next() = 0;
        virtual void previous() = 0;
};

// Pauses when currently playing, otherwise resumes playback.
void togglePlayPause(IPlaybackController& controller, PlaybackState state);

} // namespace mf
