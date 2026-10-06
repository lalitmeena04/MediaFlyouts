#pragma once

#include <QStringList>

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
        // Pause every playing session. Return source app ids of paused sessions.
        virtual QStringList pauseAllSessions() {
            return {};
        }
        // Resume sessions with the given source app ids.
        virtual void playSessions(const QStringList& appIds) {
            (void)appIds;
        }
        // Number of sessions that are playing right now.
        virtual int playingSessionCount() const {
            return 0;
        }
        // Resume every session that is not playing.
        virtual void resumeAllSessions() {}
};

// Pause if playing. Otherwise resume playback.
void togglePlayPause(IPlaybackController& controller, PlaybackState state);

} // namespace mf
