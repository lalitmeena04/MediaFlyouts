#include <gtest/gtest.h>

#include "BrowserArtwork.h"

using namespace mf;

TEST(BrowserArtworkTest, MatchesPlayingBrowserWithOneAudibleTab) {
    TrackInfo track;
    track.sourceApp = "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
    track.title = "A Song";
    track.state = PlaybackState::Playing;
    EXPECT_TRUE(matchesBrowserArtwork(track, "chrome", "a song - Music", 1));
}

TEST(BrowserArtworkTest, MatchesBrowserAppUserModelIdWithoutExeSuffix) {
    TrackInfo track;
    track.sourceApp = "Chrome";
    track.title = "A Song";
    track.state = PlaybackState::Playing;
    EXPECT_TRUE(matchesBrowserArtwork(track, "chrome", "A Song - Site", 1));
    EXPECT_FALSE(matchesBrowserArtwork(track, "msedge", "A Song - Site", 1));
}

TEST(BrowserArtworkTest, RejectsAmbiguousOrUnrelatedTabs) {
    TrackInfo track;
    track.sourceApp = "msedge.exe";
    track.title = "A Song";
    track.state = PlaybackState::Playing;
    EXPECT_FALSE(matchesBrowserArtwork(track, "msedge", "A Song", 2));
    EXPECT_FALSE(matchesBrowserArtwork(track, "msedge", "Another Song", 1));
    EXPECT_FALSE(matchesBrowserArtwork(track, "chrome", "A Song", 1));
    EXPECT_TRUE(matchesBrowserArtwork(track, "msedge", "A Song", 0));
    track.title.clear();
    EXPECT_FALSE(matchesBrowserArtwork(track, "msedge", "A Song", 1));
}

TEST(BrowserArtworkTest, KeepsPausedArtworkWhenNoTabsAreAudible) {
    TrackInfo track;
    track.sourceApp = "firefox.exe";
    track.title = "A Song";
    track.state = PlaybackState::Paused;
    EXPECT_TRUE(matchesBrowserArtwork(track, "firefox", "A Song", 0));
    EXPECT_FALSE(matchesBrowserArtwork(track, "firefox", "A Song", 1));
    track.state = PlaybackState::Stopped;
    EXPECT_FALSE(matchesBrowserArtwork(track, "firefox", "A Song", 0));
    track.state = PlaybackState::Playing;
    track.sourceApp = "other.exe";
    EXPECT_FALSE(matchesBrowserArtwork(track, "other", "A Song", 1));
}

TEST(BrowserArtworkTest, MatchesChromiumWebAppIdsWithoutAcceptingSimilarApps) {
    TrackInfo track;
    track.sourceApp = "Chrome._crx_abcdefghijklmnopabcdefghijklmnop";
    track.title = "A Song";
    track.state = PlaybackState::Playing;
    EXPECT_TRUE(matchesBrowserArtwork(track, "chrome", "A Song - Site", 1));
    track.sourceApp = "MSEdge._crx_abcdefghijklmnopabcdefghijklmnop";
    EXPECT_TRUE(matchesBrowserArtwork(track, "msedge", "A Song - Site", 1));
    EXPECT_FALSE(matchesBrowserArtwork(track, "chrome", "A Song - Site", 1));
    track.sourceApp = "Chrome._crx_not-an-id";
    EXPECT_FALSE(matchesBrowserArtwork(track, "chrome", "A Song - Site", 1));
}

TEST(BrowserArtworkTest, MatchesBraveWithoutTreatingItAsChrome) {
    TrackInfo track;
    track.sourceApp = "brave.exe";
    track.title = "A Song";
    track.state = PlaybackState::Playing;
    EXPECT_TRUE(matchesBrowserArtwork(track, "brave", "A Song - Site", 1));
    EXPECT_FALSE(matchesBrowserArtwork(track, "chrome", "A Song - Site", 1));
    track.sourceApp = "Brave";
    EXPECT_TRUE(matchesBrowserArtwork(track, "brave", "A Song - Site", 1));
}

TEST(BrowserArtworkTest, MatchesOnlyRegisteredFirefoxAppIds) {
    TrackInfo track;
    track.sourceApp = "ABCDEF1234567890";
    track.title = "Video";
    track.state = PlaybackState::Playing;
    EXPECT_FALSE(matchesBrowserArtwork(track, "firefox", "Video", 1));
    EXPECT_TRUE(matchesBrowserArtwork(track, "firefox", "Video", 1, {QStringLiteral("abcdef1234567890")}));
    EXPECT_FALSE(matchesBrowserArtwork(track, "chrome", "Video", 1, {QStringLiteral("abcdef1234567890")}));
    EXPECT_FALSE(matchesBrowserArtwork(track, "firefox", "Other", 1, {QStringLiteral("abcdef1234567890")}));
}