#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "flyoutpositioner.h"
#include "playbackcontroller.h"
#include "trackinfo.h"

using namespace mf;

TEST(FlyoutPositioner, BottomRightRespectsMargins) {
    const QRect work(0, 0, 1920, 1080);
    const QRect r = computeFlyoutRect(work, QSize(360, 120), FlyoutPosition::BottomRight, 24);
    EXPECT_EQ(r.x(), 1920 - 360 - 24);
    EXPECT_EQ(r.y(), 1080 - 120 - 24);
}

TEST(FlyoutPositioner, TopLeftRespectsMargins) {
    const QRect work(0, 0, 1920, 1080);
    const QRect r = computeFlyoutRect(work, QSize(360, 120), FlyoutPosition::TopLeft, 24);
    EXPECT_EQ(r.x(), 24);
    EXPECT_EQ(r.y(), 24);
}

TEST(TrackInfo, ProgressClampedToUnitInterval) {
    TrackInfo t;
    t.durationMs = 1000;
    t.positionMs = 5000;
    EXPECT_DOUBLE_EQ(playbackProgress(t), 1.0);
}

class MockController : public IPlaybackController {
    public:
        MOCK_METHOD(void, play, (), (override));
        MOCK_METHOD(void, pause, (), (override));
        MOCK_METHOD(void, next, (), (override));
        MOCK_METHOD(void, previous, (), (override));
};

TEST(PlaybackToggler, PausesWhenPlaying) {
    MockController controller;
    EXPECT_CALL(controller, pause()).Times(1);
    EXPECT_CALL(controller, play()).Times(0);
    togglePlayPause(controller, PlaybackState::Playing);
}

TEST(PlaybackToggler, PlaysWhenPausedOrStopped) {
    MockController controller;
    EXPECT_CALL(controller, play()).Times(2);
    togglePlayPause(controller, PlaybackState::Paused);
    togglePlayPause(controller, PlaybackState::Stopped);
}
