#include <QtTest>

#include "FlyoutPositioner.h"
#include "TrackInfo.h"

using namespace mf;

class CoreTest : public QObject {
        Q_OBJECT

    private slots:
        void bottomCenterCentersHorizontally();
        void bottomCenterSitsAboveBottomMargin();
        void formatDuration_data();
        void formatDuration();
        void progressIsClamped();
        void trackLineJoinsTitleAndArtist();
};

void CoreTest::bottomCenterCentersHorizontally() {
    const QRect work(0, 0, 1920, 1080);
    const QRect r = computeFlyoutRect(work, QSize(360, 120), FlyoutPosition::BottomCenter, 48);
    QCOMPARE(r.width(), 360);
    QCOMPARE(r.x(), (1920 - 360) / 2);
}

void CoreTest::bottomCenterSitsAboveBottomMargin() {
    const QRect work(0, 0, 1920, 1080);
    const QRect r = computeFlyoutRect(work, QSize(360, 120), FlyoutPosition::BottomCenter, 48);
    QCOMPARE(r.y(), 1080 - 120 - 48);
}

void CoreTest::formatDuration_data() {
    QTest::addColumn<qint64>("ms");
    QTest::addColumn<QString>("expected");
    QTest::newRow("zero") << qint64(0) << QString("0:00");
    QTest::newRow("65s") << qint64(65000) << QString("1:05");
    QTest::newRow("hour") << qint64(3661000) << QString("1:01:01");
    QTest::newRow("negative") << qint64(-5000) << QString("0:00");
}

void CoreTest::formatDuration() {
    QFETCH(qint64, ms);
    QFETCH(QString, expected);
    QCOMPARE(mf::formatDuration(ms), expected);
}

void CoreTest::progressIsClamped() {
    TrackInfo overrun;
    overrun.durationMs = 1000;
    overrun.positionMs = 2000;
    QCOMPARE(playbackProgress(overrun), 1.0);

    TrackInfo negative;
    negative.durationMs = 1000;
    negative.positionMs = -100;
    QCOMPARE(playbackProgress(negative), 0.0);

    TrackInfo noDuration;
    QCOMPARE(playbackProgress(noDuration), 0.0);
}

void CoreTest::trackLineJoinsTitleAndArtist() {
    TrackInfo t;
    t.title = QStringLiteral("A");
    t.artist = QStringLiteral("B");
    QCOMPARE(formatTrackLine(t), QStringLiteral("A \u2014 B"));
}

QTEST_MAIN(CoreTest)
#include "tst_core_qt.moc"
