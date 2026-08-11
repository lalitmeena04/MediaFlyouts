#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "PlaybackController.h"

using namespace mf;

namespace {
class MockController : public IPlaybackController {
    public:
        MOCK_METHOD(void, play, (), (override));
        MOCK_METHOD(void, pause, (), (override));
        MOCK_METHOD(void, next, (), (override));
        MOCK_METHOD(void, previous, (), (override));
};
} // namespace

TEST(PlaybackControllerTest, TogglePausesWhenPlaying) {
    MockController controller;
    EXPECT_CALL(controller, pause()).Times(1);
    EXPECT_CALL(controller, play()).Times(0);
    togglePlayPause(controller, PlaybackState::Playing);
}

TEST(PlaybackControllerTest, TogglePlaysWhenPaused) {
    MockController controller;
    EXPECT_CALL(controller, play()).Times(1);
    EXPECT_CALL(controller, pause()).Times(0);
    togglePlayPause(controller, PlaybackState::Paused);
}

TEST(PlaybackControllerTest, TogglePlaysWhenStopped) {
    MockController controller;
    EXPECT_CALL(controller, play()).Times(1);
    EXPECT_CALL(controller, pause()).Times(0);
    togglePlayPause(controller, PlaybackState::Stopped);
}
