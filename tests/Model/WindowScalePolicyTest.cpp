#include <gtest/gtest.h>

#include "WindowScalePolicy.h"

using namespace mf;

namespace {
WindowScaleSnapshot at(int w, int h, qreal dpr, int dpi = 0) {
    return WindowScaleSnapshot{QSize(w, h), dpr, dpi};
}

WindowScaleState state(const QSize& reported, const QSize& current, qreal dpr, int windowDpi, int screenDpi) {
    WindowScaleState s;
    s.reportedSize = reported;
    s.currentSize = current;
    s.devicePixelRatio = dpr;
    s.nativeDpi = windowDpi;
    s.screenDpi = screenDpi;
    return s;
}
} // namespace

TEST(WindowScalePolicyTest, NativeSizeScalesAndRounds) {
    EXPECT_EQ(nativeSizeOf(QSize(186, 32), 1.5), QSize(279, 48));
    EXPECT_EQ(nativeSizeOf(QSize(186, 21), 1.5), QSize(279, 32)); // 31.5 rounds up
    EXPECT_EQ(nativeSizeOf(QSize(100, 50), 0.0), QSize(100, 50)); // zero dpr is 100 %
}

TEST(WindowScalePolicyTest, SettledWhenWindowAndScreenDpiAgree) {
    EXPECT_TRUE(scaleIsSettled(96, 96));
    EXPECT_TRUE(scaleIsSettled(144, 144));
}

TEST(WindowScalePolicyTest, UnsettledWhileHiddenWindowKeepsOldDpi) {
    EXPECT_FALSE(scaleIsSettled(96, 144));
    EXPECT_FALSE(scaleIsSettled(192, 96));
}

TEST(WindowScalePolicyTest, UnknownDpiCountsAsSettled) {
    EXPECT_TRUE(scaleIsSettled(0, 144));
    EXPECT_TRUE(scaleIsSettled(96, 0));
    EXPECT_TRUE(scaleIsSettled(0, 0));
}

TEST(WindowScalePolicyTest, ResizeAtSameScaleIsAuthoritative) {
    EXPECT_TRUE(resizeIsAuthoritative(at(320, 64, 1.0, 96), state(QSize(320, 80), QSize(320, 80), 1.0, 96, 96)));
    EXPECT_TRUE(resizeIsAuthoritative(at(320, 64, 1.5, 144), state(QSize(400, 64), QSize(400, 64), 1.5, 144, 144)));
}

TEST(WindowScalePolicyTest, ResizeDuringQtScaleChangeIsNotAuthoritative) {
    EXPECT_FALSE(resizeIsAuthoritative(at(320, 64, 1.0, 96), state(QSize(480, 96), QSize(480, 96), 1.5, 144, 144)));
    EXPECT_FALSE(resizeIsAuthoritative(at(320, 64, 1.5, 144), state(QSize(213, 43), QSize(213, 43), 1.0, 96, 96)));
}

TEST(WindowScalePolicyTest, ResizeAfterNativeDpiChangeIsNotAuthoritative) {
    // Qt already bound the window to the 150 % screen while it was hidden. The
    // native window learns about 144 dpi only when shown. Same dpr, new dpi.
    EXPECT_FALSE(resizeIsAuthoritative(at(320, 64, 1.5, 96), state(QSize(480, 96), QSize(480, 96), 1.5, 144, 144)));
}

TEST(WindowScalePolicyTest, ResizeWhileUnsettledIsNotAuthoritative) {
    EXPECT_FALSE(resizeIsAuthoritative(at(320, 64, 1.5, 96), state(QSize(320, 64), QSize(320, 64), 1.5, 96, 144)));
}

TEST(WindowScalePolicyTest, StaleResizeEventIsNotAuthoritative) {
    // The event was queued at the old scale. Qt now computes another size.
    EXPECT_FALSE(resizeIsAuthoritative(at(320, 64, 1.0, 96), state(QSize(213, 43), QSize(320, 64), 1.0, 96, 96)));
}

TEST(WindowScalePolicyTest, ResizeWithoutEventHistoryIsAuthoritative) {
    EXPECT_TRUE(resizeIsAuthoritative(at(320, 64, 1.0, 96), state(QSize(), QSize(320, 80), 1.0, 96, 96)));
}

TEST(WindowScalePolicyTest, RestoresTheDoubleScaledSizeOnceSettled) {
    // Hidden on the 100 % screen, shown on the 150 % one: 320 -> 480 logical.
    const QSize restore = sizeToRestore(at(320, 64, 1.0, 96), state(QSize(480, 96), QSize(480, 96), 1.5, 144, 144));
    EXPECT_EQ(restore, QSize(320, 64));
}

TEST(WindowScalePolicyTest, RestoresTheShrunkSizeOnTheWayBack) {
    const QSize restore = sizeToRestore(at(320, 64, 1.5, 144), state(QSize(213, 43), QSize(213, 43), 1.0, 96, 96));
    EXPECT_EQ(restore, QSize(320, 64));
}

TEST(WindowScalePolicyTest, RestoresWhenOnlyTheReportedSizeIsStale) {
    const QSize restore = sizeToRestore(at(320, 64, 1.0, 96), state(QSize(213, 43), QSize(320, 64), 1.0, 96, 96));
    EXPECT_EQ(restore, QSize(320, 64));
}

TEST(WindowScalePolicyTest, WaitsWhileUnsettled) {
    EXPECT_FALSE(sizeToRestore(at(320, 64, 1.0, 96), state(QSize(213, 43), QSize(213, 43), 1.5, 96, 144)).isValid());
}

TEST(WindowScalePolicyTest, NoRestoreWhenSizeMatches) {
    EXPECT_FALSE(sizeToRestore(at(320, 64, 1.0, 96), state(QSize(320, 64), QSize(320, 64), 1.0, 96, 96)).isValid());
    EXPECT_FALSE(sizeToRestore(at(320, 64, 1.0, 96), state(QSize(320, 64), QSize(320, 64), 1.5, 144, 144)).isValid());
}

TEST(WindowScalePolicyTest, NoRestoreForOnePixelOfRounding) {
    EXPECT_FALSE(sizeToRestore(at(920, 620, 1.0, 96), state(QSize(919, 621), QSize(919, 621), 1.25, 120, 120)).isValid());
}

TEST(WindowScalePolicyTest, NoRestoreWithoutASnapshot) {
    EXPECT_FALSE(sizeToRestore(WindowScaleSnapshot{}, state(QSize(480, 96), QSize(480, 96), 1.5, 144, 144)).isValid());
}

TEST(WindowScalePolicyTest, NudgeWhenQtAlreadyHasTheTargetSize) {
    EXPECT_TRUE(needsNudge(QSize(320, 64), state(QSize(213, 43), QSize(320, 64), 1.0, 96, 96)));
}

TEST(WindowScalePolicyTest, NoNudgeWhenAResizeChangesTheSize) {
    EXPECT_FALSE(needsNudge(QSize(320, 64), state(QSize(480, 96), QSize(480, 96), 1.0, 96, 96)));
    EXPECT_FALSE(needsNudge(QSize(320, 64), state(QSize(320, 64), QSize(320, 64), 1.0, 96, 96)));
    EXPECT_FALSE(needsNudge(QSize(320, 64), state(QSize(), QSize(320, 64), 1.0, 96, 96)));
}
