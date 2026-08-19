#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "NowPlaying.h"
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

TrackInfo makeTrack(const QString& title, const QString& artist, PlaybackState state = PlaybackState::Playing) {
    TrackInfo t;
    t.title = title;
    t.artist = artist;
    t.state = state;
    return t;
}
} // namespace

TEST(NowPlayingTest, HasMediaOnlyWhenTitleOrArtistPresent) {
    NowPlaying np;
    EXPECT_FALSE(np.hasMedia());
    np.applyTrack(makeTrack(QStringLiteral("Song"), QString()));
    EXPECT_TRUE(np.hasMedia());
}

TEST(NowPlayingTest, ExposesTitleAndArtist) {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band")));
    EXPECT_EQ(np.title(), QStringLiteral("Song"));
    EXPECT_EQ(np.artist(), QStringLiteral("Band"));
}

TEST(NowPlayingTest, FirstUpdateDoesNotRaiseFlyoutSignals) {
    NowPlaying np;
    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);
    QSignalSpy changed(&np, &NowPlaying::changed);

    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band")));

    EXPECT_EQ(trackUpdated.count(), 0);
    EXPECT_EQ(nextUp.count(), 0);
    EXPECT_EQ(changed.count(), 1);
}

TEST(NowPlayingTest, AutoAdvanceRaisesNextUp) {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("First"), QStringLiteral("Band")));

    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);

    np.applyTrack(makeTrack(QStringLiteral("Second"), QStringLiteral("Band")));

    EXPECT_EQ(nextUp.count(), 1);
    EXPECT_EQ(trackUpdated.count(), 0);
}

TEST(NowPlayingTest, ManualSkipRaisesTrackUpdated) {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("First"), QStringLiteral("Band")));

    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);

    np.noteMediaKey(); // a key press just before the track change means a manual skip
    np.applyTrack(makeTrack(QStringLiteral("Second"), QStringLiteral("Band")));

    EXPECT_EQ(trackUpdated.count(), 1);
    EXPECT_EQ(nextUp.count(), 0);
}

TEST(NowPlayingTest, PlayPauseStateChangeRaisesTrackUpdated) {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));

    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Paused));

    EXPECT_EQ(trackUpdated.count(), 1);
}

TEST(NowPlayingTest, StoppingRaisesTrackUpdatedNotNextUp) {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band")));

    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);

    np.applyTrack(makeTrack(QString(), QString(), PlaybackState::Stopped));

    EXPECT_EQ(trackUpdated.count(), 1);
    EXPECT_EQ(nextUp.count(), 0);
}

TEST(NowPlayingTest, PlayPauseForwardsToggleToController) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));

    EXPECT_CALL(controller, pause()).Times(1);
    np.playPause();
}

TEST(NowPlayingTest, NextAndPreviousForwardToController) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);

    EXPECT_CALL(controller, next()).Times(1);
    EXPECT_CALL(controller, previous()).Times(1);
    np.next();
    np.previous();
}

TEST(NowPlayingTest, TransportCommandsAreSafeWithoutController) {
    NowPlaying np;
    np.playPause();
    np.next();
    np.previous();
    SUCCEED();
}

TEST(NowPlayingTest, ApplyArtTogglesHasArtAndArtSource) {
    NowPlaying np;
    EXPECT_FALSE(np.hasArt());
    EXPECT_TRUE(np.artSource().isEmpty());

    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    np.applyArt(img);

    EXPECT_TRUE(np.hasArt());
    EXPECT_FALSE(np.artSource().isEmpty());

    np.applyArt(QImage());
    EXPECT_FALSE(np.hasArt());
}

TEST(NowPlayingTest, ArtAccentReflectsDominantColor) {
    NowPlaying np;
    // default accent before any art
    EXPECT_EQ(np.artAccent(), QColor("#4CC2FF"));

    QImage red(8, 8, QImage::Format_ARGB32);
    red.fill(Qt::red);
    np.applyArt(red);
    const QColor accent = np.artAccent();
    EXPECT_GT(accent.red(), accent.green());
    EXPECT_GT(accent.red(), accent.blue());

    // clearing art restores the default accent
    np.applyArt(QImage());
    EXPECT_EQ(np.artAccent(), QColor("#4CC2FF"));
}
