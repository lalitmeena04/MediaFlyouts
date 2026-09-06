#pragma once

#include "TrackInfo.h"

namespace mf {

// Media transport abstraction. Playback logic stays testable.
class IPlaybackController {
    public:
        virtual ~IPlaybackController() = default;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void next() = 0;
        virtual void previous() = 0;
        // Optional transport extras. No-op if provider lacks support.
        virtual void seek(qint64 positionMs) {
            (void)positionMs;
        }
        virtual void setRepeatMode(int mode) {
            (void)mode;
        }
        virtual void setShuffle(bool on) {
            (void)on;
        }
        // Number of detected media sessions. Default is 1.
        virtual int sessionCount() const {
            return 1;
        }
        // Index of displayed session in session list.
        virtual int selectedSessionIndex() const {
            return 0;
        }
        // Move displayed session by delta.
        virtual void cycleSession(int delta) {
            (void)delta;
        }
};

// Pause if playing. Otherwise resume playback.
void togglePlayPause(IPlaybackController& controller, PlaybackState state);

} // namespace mf
