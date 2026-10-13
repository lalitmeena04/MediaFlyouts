#pragma once

#include <QStringList>

#include "TrackInfo.h"

namespace mf {

class IPlaybackController {
    public:
        virtual ~IPlaybackController() = default;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void next() = 0;
        virtual void previous() = 0;
        virtual void seek(qint64 positionMs) {
            (void)positionMs;
        }
        virtual void setRepeatMode(int mode) {
            (void)mode;
        }
        virtual void setShuffle(bool on) {
            (void)on;
        }
        virtual int sessionCount() const {
            return 1;
        }
        virtual int selectedSessionIndex() const {
            return 0;
        }
        virtual void cycleSession(int delta) {
            (void)delta;
        }
        virtual QStringList pauseAllSessions() {
            return {};
        }
        virtual int playingSessionCount() const {
            return 0;
        }
        virtual void resumeAllSessions() {}
};

void togglePlayPause(IPlaybackController& controller, PlaybackState state);

} // namespace mf
