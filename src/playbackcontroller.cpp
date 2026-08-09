#include "playbackcontroller.h"

namespace mf {

void togglePlayPause(IPlaybackController& controller, PlaybackState state)
{
    if (state == PlaybackState::Playing)
        controller.pause();
    else
        controller.play();
}

} // namespace mf
