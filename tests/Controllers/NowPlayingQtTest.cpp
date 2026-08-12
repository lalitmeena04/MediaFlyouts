#include <QtTest>

#include "NowPlaying.h"
#include "PlaybackController.h"

using namespace mf;

namespace {
class SpyController : public IPlaybackController {
    public:
        int plays = 0;
        int pauses = 0;
        int nexts = 0;
        int prevs = 0;
        void play() override {
            ++plays;
        }
        void pause() override {
            ++pauses;
        }
        void next() override {
            ++nexts;
        }
        void previous() override {
            ++prevs;
        }
};

TrackInfo makeTrack(const QString& title, const QString& artist, PlaybackState state = PlaybackState::Playing) {
    TrackInfo t;
    t.title = title;
    t.artist = artist;
    t.state = state;
    return t;
}
} // namespace

class NowPlayingQtTest : public QObject {
        Q_OBJECT

    private slots:
        void hasMediaReflectsTrack();
        void firstUpdateIsSilent();
        void autoAdvanceRaisesNextUp();
        void manualSkipRaisesTrackUpdated();
        void stateChangeRaisesTrackUpdated();
        void stoppingRaisesTrackUpdated();
        void transportForwardsToController();
        void transportSafeWithoutController();
        void applyArtTogglesHasArt();
};

void NowPlayingQtTest::hasMediaReflectsTrack() {
    NowPlaying np;
    QVERIFY(!np.hasMedia());
    np.applyTrack(makeTrack(QStringLiteral("Song"), QString()));
    QVERIFY(np.hasMedia());
    QCOMPARE(np.title(), QStringLiteral("Song"));
}

void NowPlayingQtTest::firstUpdateIsSilent() {
    NowPlaying np;
    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band")));
    QCOMPARE(trackUpdated.count(), 0);
    QCOMPARE(nextUp.count(), 0);
}

void NowPlayingQtTest::autoAdvanceRaisesNextUp() {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("First"), QStringLiteral("Band")));
    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);
    np.applyTrack(makeTrack(QStringLiteral("Second"), QStringLiteral("Band")));
    QCOMPARE(nextUp.count(), 1);
    QCOMPARE(trackUpdated.count(), 0);
}

void NowPlayingQtTest::manualSkipRaisesTrackUpdated() {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("First"), QStringLiteral("Band")));
    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);
    np.noteMediaKey();
    np.applyTrack(makeTrack(QStringLiteral("Second"), QStringLiteral("Band")));
    QCOMPARE(trackUpdated.count(), 1);
    QCOMPARE(nextUp.count(), 0);
}

void NowPlayingQtTest::stateChangeRaisesTrackUpdated() {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));
    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Paused));
    QCOMPARE(trackUpdated.count(), 1);
}

void NowPlayingQtTest::stoppingRaisesTrackUpdated() {
    NowPlaying np;
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band")));
    QSignalSpy trackUpdated(&np, &NowPlaying::trackUpdated);
    QSignalSpy nextUp(&np, &NowPlaying::nextUpRequested);
    np.applyTrack(makeTrack(QString(), QString(), PlaybackState::Stopped));
    QCOMPARE(trackUpdated.count(), 1);
    QCOMPARE(nextUp.count(), 0);
}

void NowPlayingQtTest::transportForwardsToController() {
    NowPlaying np;
    SpyController controller;
    np.setController(&controller);
    np.applyTrack(makeTrack(QStringLiteral("Song"), QStringLiteral("Band"), PlaybackState::Playing));
    np.playPause();
    np.next();
    np.previous();
    QCOMPARE(controller.pauses, 1);
    QCOMPARE(controller.nexts, 1);
    QCOMPARE(controller.prevs, 1);
}

void NowPlayingQtTest::transportSafeWithoutController() {
    NowPlaying np;
    np.playPause();
    np.next();
    np.previous();
    QVERIFY(true);
}

void NowPlayingQtTest::applyArtTogglesHasArt() {
    NowPlaying np;
    QVERIFY(!np.hasArt());
    QVERIFY(np.artSource().isEmpty());
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    np.applyArt(img);
    QVERIFY(np.hasArt());
    QVERIFY(!np.artSource().isEmpty());
    np.applyArt(QImage());
    QVERIFY(!np.hasArt());
}

QTEST_GUILESS_MAIN(NowPlayingQtTest)
#include "NowPlayingQtTest.moc"
