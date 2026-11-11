#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "AlbumArtProvider.h"
#include "BrowserArtworkImage.h"
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
        MOCK_METHOD(void, seek, (qint64), (override));
        MOCK_METHOD(void, setRepeatMode, (int), (override));
        MOCK_METHOD(void, setShuffle, (bool), (override));
        MOCK_METHOD(int, sessionCount, (), (const, override));
        MOCK_METHOD(int, selectedSessionIndex, (), (const, override));
        MOCK_METHOD(void, cycleSession, (int), (override));
        MOCK_METHOD(QStringList, pauseAllSessions, (), (override));
        MOCK_METHOD(int, playingSessionCount, (), (const, override));
        MOCK_METHOD(void, resumeAllSessions, (), (override));
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

    np.noteMediaKey();
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
    np.pauseAll();
    np.playAll();
    EXPECT_FALSE(np.canResumeAll());
    SUCCEED();
}

TEST(NowPlayingTest, PauseAllRemembersPausedSessions) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    QSignalSpy resumeAll(&np, &NowPlaying::resumeAllChanged);
    QSignalSpy pauseBrowsers(&np, &NowPlaying::pauseAllBrowsersRequested);

    const QStringList paused{QStringLiteral("Spotify.exe"), QStringLiteral("chrome.exe")};
    EXPECT_CALL(controller, pauseAllSessions()).WillOnce(::testing::Return(paused));
    np.pauseAll();

    EXPECT_TRUE(np.canResumeAll());
    EXPECT_EQ(resumeAll.count(), 1);
    EXPECT_EQ(pauseBrowsers.count(), 1);
}

TEST(NowPlayingTest, PauseAllRunsWindowsFallbackBeforeBrowserExtensionCommand) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    QStringList calls;

    EXPECT_CALL(controller, pauseAllSessions())
        .WillOnce(::testing::Invoke([&calls] {
            calls.push_back(QStringLiteral("windows"));
            return QStringList{QStringLiteral("chrome.exe")};
        }));
    QObject::connect(&np, &NowPlaying::pauseAllBrowsersRequested, &np, [&calls] {
        calls.push_back(QStringLiteral("browser"));
    });

    np.pauseAll();

    EXPECT_EQ(calls, (QStringList{QStringLiteral("windows"), QStringLiteral("browser")}));
}

TEST(NowPlayingTest, PauseAllWithNothingPlayingLeavesNothingToResume) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);

    EXPECT_CALL(controller, pauseAllSessions()).WillOnce(::testing::Return(QStringList()));
    np.pauseAll();

    EXPECT_FALSE(np.canResumeAll());
}

TEST(NowPlayingTest, PlayAllResumesEveryPausedSessionThenClears) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);

    const QStringList paused{QStringLiteral("Spotify.exe")};
    EXPECT_CALL(controller, pauseAllSessions()).WillOnce(::testing::Return(paused));
    np.pauseAll();

    QSignalSpy resumeAll(&np, &NowPlaying::resumeAllChanged);
    EXPECT_CALL(controller, resumeAllSessions()).Times(1);
    np.playAll();

    EXPECT_FALSE(np.canResumeAll());
    EXPECT_EQ(resumeAll.count(), 1);

    EXPECT_CALL(controller, resumeAllSessions()).Times(0);
    np.playAll();
}

TEST(NowPlayingTest, AllSessionsPausedByHandOffersPlayAll) {
    NowPlaying np;
    ::testing::NiceMock<MockController> controller;
    np.setController(&controller);
    ON_CALL(controller, playingSessionCount()).WillByDefault(::testing::Return(0));
    QSignalSpy resumeAll(&np, &NowPlaying::resumeAllChanged);

    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Paused));
    EXPECT_TRUE(np.canResumeAll());
    EXPECT_EQ(resumeAll.count(), 1);

    EXPECT_CALL(controller, resumeAllSessions()).Times(1);
    np.playAll();
}

TEST(NowPlayingTest, PlayAllHiddenWhileAnotherSessionStillPlays) {
    NowPlaying np;
    ::testing::NiceMock<MockController> controller;
    np.setController(&controller);
    int playing = 1;
    ON_CALL(controller, playingSessionCount()).WillByDefault(::testing::Invoke([&playing] { return playing; }));

    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Paused));
    EXPECT_FALSE(np.canResumeAll());

    QSignalSpy resumeAll(&np, &NowPlaying::resumeAllChanged);
    playing = 0;
    np.noteSessionsChanged();
    EXPECT_TRUE(np.canResumeAll());
    EXPECT_EQ(resumeAll.count(), 1);
}

TEST(NowPlayingTest, PlayAllNotOfferedWithoutMedia) {
    NowPlaying np;
    ::testing::NiceMock<MockController> controller;
    np.setController(&controller);
    ON_CALL(controller, playingSessionCount()).WillByDefault(::testing::Return(0));

    np.applyTrack(makeTrack(QString(), QString(), PlaybackState::Stopped));
    EXPECT_FALSE(np.canResumeAll());
}

TEST(NowPlayingTest, ManualResumeForgetsPauseAllSet) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));

    EXPECT_CALL(controller, pauseAllSessions()).WillOnce(::testing::Return(QStringList{QStringLiteral("Spotify.exe")}));
    np.pauseAll();
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Paused));
    EXPECT_TRUE(np.canResumeAll());

    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));
    EXPECT_FALSE(np.canResumeAll());
}

TEST(NowPlayingTest, StalePlayingUpdateAfterPauseAllKeepsSet) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));

    EXPECT_CALL(controller, pauseAllSessions()).WillOnce(::testing::Return(QStringList{QStringLiteral("Spotify.exe")}));
    np.pauseAll();

    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));
    EXPECT_TRUE(np.canResumeAll());
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

TEST(NowPlayingTest, EmptyTrackWithNullArtClearsEveryArtFacet) {
    NowPlaying np;
    AlbumArtProvider provider;
    np.setArtProvider(&provider);

    QImage red(8, 8, QImage::Format_ARGB32);
    red.fill(Qt::red);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band")));
    np.applyArt(red);
    ASSERT_TRUE(np.hasMedia());
    ASSERT_TRUE(np.hasArt());

    QSize size;
    np.applyTrack(TrackInfo());
    np.applyArt(QImage());

    EXPECT_FALSE(np.hasMedia());
    EXPECT_FALSE(np.hasArt());
    EXPECT_TRUE(np.artSource().isEmpty());
    EXPECT_EQ(np.artAccent(), QColor("#4CC2FF"));
    EXPECT_TRUE(provider.requestImage(QStringLiteral("cover"), &size, QSize()).isNull());
}

TEST(NowPlayingTest, ArtAccentReflectsDominantColor) {
    NowPlaying np;
    EXPECT_EQ(np.artAccent(), QColor("#4CC2FF"));

    QImage red(8, 8, QImage::Format_ARGB32);
    red.fill(Qt::red);
    np.applyArt(red);
    const QColor accent = np.artAccent();
    EXPECT_GT(accent.red(), accent.green());
    EXPECT_GT(accent.red(), accent.blue());

    np.applyArt(QImage());
    EXPECT_EQ(np.artAccent(), QColor("#4CC2FF"));
}

TEST(NowPlayingTest, BrowserArtworkIsOnlyUsedForMatchingMediaWithoutAlbumArt) {
    NowPlaying np;
    AlbumArtProvider provider;
    np.setArtProvider(&provider);
    TrackInfo track = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing);
    track.sourceApp = QStringLiteral("chrome.exe");
    np.applyTrack(track);

    QImage favicon(4, 4, QImage::Format_ARGB32);
    favicon.fill(Qt::blue);
    np.applyBrowserArtwork(QStringLiteral("chrome"), QStringLiteral("Song - Site"), 1, favicon);
    QSize size;
    EXPECT_EQ(provider.requestImage(QStringLiteral("cover"), &size, QSize()).pixelColor(0, 0), QColor(Qt::blue));

    QImage album(4, 4, QImage::Format_ARGB32);
    album.fill(Qt::red);
    np.applyArt(album);
    EXPECT_EQ(provider.requestImage(QStringLiteral("cover"), &size, QSize()).pixelColor(0, 0), QColor(Qt::red));
    np.applyArt(QImage());
    EXPECT_EQ(provider.requestImage(QStringLiteral("cover"), &size, QSize()).pixelColor(0, 0), QColor(Qt::blue));

    track.title = QStringLiteral("Other");
    np.applyTrack(track);
    EXPECT_FALSE(np.hasArt());
    np.applyBrowserArtwork(QStringLiteral("chrome"), QStringLiteral("Other"), 2, favicon);
    EXPECT_FALSE(np.hasArt());
}

TEST(NowPlayingTest, BrowserArtworkStaysWhenPausedAndClearsOnDisconnect) {
    NowPlaying np;
    TrackInfo track = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing);
    track.sourceApp = QStringLiteral("firefox.exe");
    np.applyTrack(track);
    QImage favicon(4, 4, QImage::Format_ARGB32);
    favicon.fill(Qt::blue);
    np.applyBrowserArtwork(QStringLiteral("firefox"), QStringLiteral("Song"), 1, favicon);
    EXPECT_TRUE(np.hasArt());
    track.state = PlaybackState::Paused;
    np.applyTrack(track);
    np.applyBrowserArtwork(QStringLiteral("firefox"), QStringLiteral("Song"), 0, favicon);
    EXPECT_TRUE(np.hasArt());
    track.state = PlaybackState::Playing;
    np.applyTrack(track);
    EXPECT_TRUE(np.hasArt());
    np.clearBrowserArtwork(QStringLiteral("firefox"));
    EXPECT_FALSE(np.hasArt());
}

TEST(NowPlayingTest, BrowserArtworkReplacesOnlyMatchingBrowserIcons) {
    QImage browserIcon(32, 32, QImage::Format_ARGB32);
    browserIcon.fill(Qt::transparent);
    for(int row = 8; row < 24; ++row)
        for(int column = 8; column < 24; ++column)
            browserIcon.setPixelColor(column, row, Qt::blue);
    const QImage thumbnail = browserIcon.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage favicon(16, 16, QImage::Format_ARGB32);
    favicon.fill(Qt::red);
    QImage album(256, 256, QImage::Format_ARGB32);
    album.fill(Qt::green);
    EXPECT_EQ(mf::chooseBrowserArtwork(thumbnail, favicon, browserIcon), favicon);
    EXPECT_EQ(mf::chooseBrowserArtwork(album, favicon, browserIcon), album);
    EXPECT_EQ(mf::chooseBrowserArtwork(album, favicon, QImage()), album);
    EXPECT_EQ(mf::chooseBrowserArtwork(thumbnail, QImage(), browserIcon), thumbnail);
    EXPECT_EQ(mf::chooseBrowserArtwork(QImage(), favicon, QImage()), favicon);
}

TEST(NowPlayingTest, SeekToFractionForwardsClampedPosition) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    TrackInfo t = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"));
    t.durationMs = 200000;
    t.canSeek = true;
    np.applyTrack(t);

    EXPECT_CALL(controller, seek(100000)).Times(1);
    np.seekToFraction(0.5);

    EXPECT_CALL(controller, seek(0)).Times(1);
    np.seekToFraction(-1.0); // clamps below zero

    EXPECT_CALL(controller, seek(200000)).Times(1);
    np.seekToFraction(2.0); // clamps to the duration
}

TEST(NowPlayingTest, SeekToFractionNoOpWithoutDuration) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band")));

    EXPECT_CALL(controller, seek(::testing::_)).Times(0);
    np.seekToFraction(0.5);
}

TEST(NowPlayingTest, CycleRepeatAdvancesModuloThree) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);

    TrackInfo t = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"));
    t.repeatMode = 0;
    np.applyTrack(t);
    EXPECT_CALL(controller, setRepeatMode(1)).Times(1);
    np.cycleRepeat();

    t.repeatMode = 2;
    np.applyTrack(t);
    EXPECT_CALL(controller, setRepeatMode(0)).Times(1); // 2 -> 0 wraps around
    np.cycleRepeat();
}

TEST(NowPlayingTest, ToggleShuffleInvertsCurrentState) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);

    TrackInfo t = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"));
    t.shuffle = false;
    np.applyTrack(t);
    EXPECT_CALL(controller, setShuffle(true)).Times(1);
    np.toggleShuffle();

    t.shuffle = true;
    np.applyTrack(t);
    EXPECT_CALL(controller, setShuffle(false)).Times(1);
    np.toggleShuffle();
}

TEST(NowPlayingTest, CycleSessionForwardsDelta) {
    NowPlaying np;
    MockController controller;
    np.setController(&controller);
    EXPECT_CALL(controller, cycleSession(1)).Times(1);
    EXPECT_CALL(controller, cycleSession(-1)).Times(1);
    np.cycleSession(1);
    np.cycleSession(-1);
}

TEST(NowPlayingTest, SessionCountAndIndexReflectController) {
    NowPlaying np;
    MockController controller;
    ON_CALL(controller, sessionCount()).WillByDefault(::testing::Return(3));
    ON_CALL(controller, selectedSessionIndex()).WillByDefault(::testing::Return(2));
    np.setController(&controller);
    EXPECT_EQ(np.sessionCount(), 3);
    EXPECT_EQ(np.selectedSessionIndex(), 2);
}

TEST(NowPlayingTest, SessionCountDefaultsToOneWithoutController) {
    NowPlaying np;
    EXPECT_EQ(np.sessionCount(), 1);
    EXPECT_EQ(np.selectedSessionIndex(), 0);
}

TEST(NowPlayingTest, TransportExtrasAreSafeWithoutController) {
    NowPlaying np;
    np.seekToFraction(0.5);
    np.cycleRepeat();
    np.toggleShuffle();
    np.cycleSession(1);
    SUCCEED();
}

TEST(NowPlayingTest, PlayerNameStripsExeSuffix) {
    NowPlaying np;
    TrackInfo t = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"));
    t.sourceApp = QStringLiteral("Spotify.exe");
    np.applyTrack(t);
    EXPECT_EQ(np.playerName(), QStringLiteral("Spotify"));
}

TEST(NowPlayingTest, PlayerNameExtractsAppIdFromAumid) {
    NowPlaying np;
    TrackInfo t = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"));
    t.sourceApp = QStringLiteral("Microsoft.ZuneMusic_8wekyb3d8bbwe!Microsoft.ZuneMusic");
    np.applyTrack(t);
    EXPECT_EQ(np.playerName(), QStringLiteral("Microsoft.ZuneMusic"));
}

TEST(NowPlayingTest, ReflectsRepeatShuffleAndCapabilities) {
    NowPlaying np;
    TrackInfo t = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"));
    t.repeatMode = 2;
    t.shuffle = true;
    t.canSeek = true;
    t.canRepeat = true;
    t.canShuffle = true;
    np.applyTrack(t);
    EXPECT_EQ(np.repeatMode(), 2);
    EXPECT_TRUE(np.shuffle());
    EXPECT_TRUE(np.canSeek());
    EXPECT_TRUE(np.canRepeat());
    EXPECT_TRUE(np.canShuffle());
}

TEST(NowPlayingTest, ElapsedDurationAndProgressFromPausedPosition) {
    NowPlaying np;
    TrackInfo t = makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Paused);
    t.positionMs = 65000;  // 1:05
    t.durationMs = 200000; // 3:20
    np.applyTrack(t);
    EXPECT_EQ(np.elapsed(), QStringLiteral("1:05"));
    EXPECT_EQ(np.duration(), QStringLiteral("3:20"));
    EXPECT_NEAR(np.progress(), 0.325, 1e-3);
    EXPECT_FALSE(np.isPlaying());
}
