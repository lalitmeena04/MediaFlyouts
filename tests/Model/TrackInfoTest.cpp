#include <gtest/gtest.h>

#include "TrackInfo.h"

using namespace mf;

TEST(TrackInfoTest, FormatDurationZero) {
    EXPECT_EQ(formatDuration(0), QStringLiteral("0:00"));
}

TEST(TrackInfoTest, FormatDurationSeconds) {
    EXPECT_EQ(formatDuration(5000), QStringLiteral("0:05"));
}

TEST(TrackInfoTest, FormatDurationMinutesAndSeconds) {
    EXPECT_EQ(formatDuration(65000), QStringLiteral("1:05"));
}

TEST(TrackInfoTest, FormatDurationWithHours) {
    EXPECT_EQ(formatDuration(3661000), QStringLiteral("1:01:01"));
}

TEST(TrackInfoTest, FormatDurationNegativeClampsToZero) {
    EXPECT_EQ(formatDuration(-5000), QStringLiteral("0:00"));
}

TEST(TrackInfoTest, FormatTrackLineJoinsTitleAndArtist) {
    TrackInfo t;
    t.title = QStringLiteral("Song");
    t.artist = QStringLiteral("Band");
    EXPECT_EQ(formatTrackLine(t), QStringLiteral("Song \u2014 Band"));
}

TEST(TrackInfoTest, FormatTrackLineTitleOnly) {
    TrackInfo t;
    t.title = QStringLiteral("Song");
    EXPECT_EQ(formatTrackLine(t), QStringLiteral("Song"));
}

TEST(TrackInfoTest, FormatTrackLineArtistOnly) {
    TrackInfo t;
    t.artist = QStringLiteral("Band");
    EXPECT_EQ(formatTrackLine(t), QStringLiteral("Band"));
}

TEST(TrackInfoTest, FormatTrackLineEmpty) {
    EXPECT_TRUE(formatTrackLine(TrackInfo{}).isEmpty());
}

TEST(TrackInfoTest, ProgressMidwayIsHalf) {
    TrackInfo t;
    t.durationMs = 1000;
    t.positionMs = 500;
    EXPECT_DOUBLE_EQ(playbackProgress(t), 0.5);
}

TEST(TrackInfoTest, ProgressClampsOverrunToOne) {
    TrackInfo t;
    t.durationMs = 1000;
    t.positionMs = 5000;
    EXPECT_DOUBLE_EQ(playbackProgress(t), 1.0);
}

TEST(TrackInfoTest, ProgressClampsNegativeToZero) {
    TrackInfo t;
    t.durationMs = 1000;
    t.positionMs = -100;
    EXPECT_DOUBLE_EQ(playbackProgress(t), 0.0);
}

TEST(TrackInfoTest, ProgressWithoutDurationIsZero) {
    EXPECT_DOUBLE_EQ(playbackProgress(TrackInfo{}), 0.0);
}
