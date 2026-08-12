#include <QtTest>

#include "PlaybackController.h"

using namespace mf;

namespace {
// Records which transport calls happen so the toggle logic can be asserted.
class SpyController : public IPlaybackController {
    public:
        int plays = 0;
        int pauses = 0;
        void play() override {
            ++plays;
        }
        void pause() override {
            ++pauses;
        }
        void next() override {}
        void previous() override {}
};
} // namespace

class PlaybackControllerQtTest : public QObject {
        Q_OBJECT

    private slots:
        void toggle_data();
        void toggle();
};

void PlaybackControllerQtTest::toggle_data() {
    QTest::addColumn<int>("state");
    QTest::addColumn<int>("plays");
    QTest::addColumn<int>("pauses");
    QTest::newRow("playing-pauses") << int(PlaybackState::Playing) << 0 << 1;
    QTest::newRow("paused-plays") << int(PlaybackState::Paused) << 1 << 0;
    QTest::newRow("stopped-plays") << int(PlaybackState::Stopped) << 1 << 0;
}

void PlaybackControllerQtTest::toggle() {
    QFETCH(int, state);
    QFETCH(int, plays);
    QFETCH(int, pauses);
    SpyController controller;
    togglePlayPause(controller, PlaybackState(state));
    QCOMPARE(controller.plays, plays);
    QCOMPARE(controller.pauses, pauses);
}

QTEST_GUILESS_MAIN(PlaybackControllerQtTest)
#include "PlaybackControllerQtTest.moc"
