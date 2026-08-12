#include <QtTest>

#include "TrackInfo.h"

using namespace mf;

class TrackInfoQtTest : public QObject {
        Q_OBJECT

    private slots:
        void formatDuration_data();
        void formatDuration();
        void formatTrackLine_data();
        void formatTrackLine();
        void playbackProgress_data();
        void playbackProgress();
};

void TrackInfoQtTest::formatDuration_data() {
    QTest::addColumn<qint64>("ms");
    QTest::addColumn<QString>("expected");
    QTest::newRow("zero") << qint64(0) << QString("0:00");
    QTest::newRow("seconds") << qint64(5000) << QString("0:05");
    QTest::newRow("minutes") << qint64(65000) << QString("1:05");
    QTest::newRow("hours") << qint64(3661000) << QString("1:01:01");
    QTest::newRow("negative") << qint64(-5000) << QString("0:00");
}

void TrackInfoQtTest::formatDuration() {
    QFETCH(qint64, ms);
    QFETCH(QString, expected);
    QCOMPARE(mf::formatDuration(ms), expected);
}

void TrackInfoQtTest::formatTrackLine_data() {
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("artist");
    QTest::addColumn<QString>("expected");
    QTest::newRow("both") << "Song" << "Band" << QString("Song \u2014 Band");
    QTest::newRow("title-only") << "Song" << "" << QString("Song");
    QTest::newRow("artist-only") << "" << "Band" << QString("Band");
    QTest::newRow("empty") << "" << "" << QString("");
}

void TrackInfoQtTest::formatTrackLine() {
    QFETCH(QString, title);
    QFETCH(QString, artist);
    QFETCH(QString, expected);
    TrackInfo t;
    t.title = title;
    t.artist = artist;
    QCOMPARE(mf::formatTrackLine(t), expected);
}

void TrackInfoQtTest::playbackProgress_data() {
    QTest::addColumn<qint64>("duration");
    QTest::addColumn<qint64>("position");
    QTest::addColumn<double>("expected");
    QTest::newRow("half") << qint64(1000) << qint64(500) << 0.5;
    QTest::newRow("overrun") << qint64(1000) << qint64(5000) << 1.0;
    QTest::newRow("negative") << qint64(1000) << qint64(-100) << 0.0;
    QTest::newRow("no-duration") << qint64(0) << qint64(500) << 0.0;
}

void TrackInfoQtTest::playbackProgress() {
    QFETCH(qint64, duration);
    QFETCH(qint64, position);
    QFETCH(double, expected);
    TrackInfo t;
    t.durationMs = duration;
    t.positionMs = position;
    QCOMPARE(mf::playbackProgress(t), expected);
}

QTEST_GUILESS_MAIN(TrackInfoQtTest)
#include "TrackInfoQtTest.moc"
