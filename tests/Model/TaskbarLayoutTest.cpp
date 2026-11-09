#include <gtest/gtest.h>

#include "TaskbarLayout.h"

using mf::kUnmeasuredTrayWidth;
using mf::reserveOnScreen;
using mf::TaskbarReserve;
using mf::TaskbarSide;
using mf::taskbarWidgetX;
using mf::detectTaskbarEdge;
using mf::isHorizontalEdge;
using mf::kDefaultTaskbarDepth;
using mf::TaskbarEdge;
using mf::taskbarWidgetY;

namespace {

constexpr int kMargin = 8;
constexpr int kWidth = 220;

QRect secondaryScreen() {
    return QRect(1920, 0, 2560, 1440);
}

QRect primaryScreen() {
    return QRect(0, 0, 1920, 1080);
}

} // namespace

TEST(TaskbarLayoutTest, ReserveOffTheScreenIsDropped) {
    TaskbarReserve reserve;
    reserve.leading = QRect(0, 1040, 48, 40);
    reserve.trailing = QRect(1600, 1040, 320, 40);

    const TaskbarReserve kept = reserveOnScreen(primaryScreen(), reserve);
    EXPECT_EQ(kept.leading, reserve.leading);
    EXPECT_EQ(kept.trailing, reserve.trailing);

    const TaskbarReserve dropped = reserveOnScreen(secondaryScreen(), reserve);
    EXPECT_TRUE(dropped.leading.isEmpty());
    EXPECT_TRUE(dropped.trailing.isEmpty());
}

TEST(TaskbarLayoutTest, ForeignTrayDoesNotPinTheWidgetToTheEdge) {
    const QRect screen(0, 0, 800, 600);
    TaskbarReserve foreign;
    foreign.trailing = QRect(2300, 1400, 260, 40);

    const int x = taskbarWidgetX(screen, reserveOnScreen(screen, foreign), TaskbarSide::Right, 0, kWidth, kMargin);
    EXPECT_EQ(x, 800 - kUnmeasuredTrayWidth - kWidth - kMargin);
    EXPECT_LT(x + kWidth, screen.right() + 1);
}

TEST(TaskbarLayoutTest, RightSideSitsBesideTheMeasuredTray) {
    TaskbarReserve reserve;
    reserve.trailing = QRect(1600, 1040, 320, 40);

    const int x = taskbarWidgetX(primaryScreen(), reserve, TaskbarSide::Right, 0, kWidth, kMargin);
    EXPECT_EQ(x, 1600 - kWidth - kMargin);
}

TEST(TaskbarLayoutTest, EachScreenUsesItsOwnTrayWidth) {
    const QRect screen = secondaryScreen();

    TaskbarReserve narrow;
    narrow.trailing = QRect(4360, 1400, 120, 40); // small clock-only tray
    TaskbarReserve wide;
    wide.trailing = QRect(4000, 1400, 480, 40); // full tray

    const int narrowX = taskbarWidgetX(screen, narrow, TaskbarSide::Right, 0, kWidth, kMargin);
    const int wideX = taskbarWidgetX(screen, wide, TaskbarSide::Right, 0, kWidth, kMargin);

    EXPECT_EQ(narrowX, 4360 - kWidth - kMargin);
    EXPECT_EQ(wideX, 4000 - kWidth - kMargin);
    EXPECT_GT(narrowX, wideX);
}

TEST(TaskbarLayoutTest, RightSideFallsBackWhenTheTrayIsNotMeasured) {
    const QRect screen = secondaryScreen();

    const int x = taskbarWidgetX(screen, TaskbarReserve{}, TaskbarSide::Right, 0, kWidth, kMargin);
    EXPECT_EQ(x, screen.right() + 1 - kUnmeasuredTrayWidth - kWidth - kMargin);
}

TEST(TaskbarLayoutTest, IgnoresChromeThatDoesNotTouchItsScreenEdge) {
    const QRect screen = primaryScreen();

    TaskbarReserve centred;
    centred.trailing = QRect(800, 1040, 320, 40);
    centred.leading = QRect(800, 1040, 320, 40);

    EXPECT_EQ(taskbarWidgetX(screen, centred, TaskbarSide::Right, 0, kWidth, kMargin),
              screen.right() + 1 - kUnmeasuredTrayWidth - kWidth - kMargin);
    EXPECT_EQ(taskbarWidgetX(screen, centred, TaskbarSide::Left, 0, kWidth, kMargin), screen.left() + kMargin);
}

TEST(TaskbarLayoutTest, LeftSideClearsStartButtonWhenItHoldsTheEdge) {
    const QRect screen = secondaryScreen();

    TaskbarReserve reserve;
    reserve.leading = QRect(1920, 1400, 60, 40);

    const int x = taskbarWidgetX(screen, reserve, TaskbarSide::Left, 0, kWidth, kMargin);
    EXPECT_EQ(x, 1980 + kMargin);
}

TEST(TaskbarLayoutTest, CentersOnTheOwningScreen) {
    const QRect screen = secondaryScreen();

    const int x = taskbarWidgetX(screen, TaskbarReserve{}, TaskbarSide::Center, 0, kWidth, kMargin);
    EXPECT_EQ(x, 1920 + (2560 - kWidth) / 2);
}

TEST(TaskbarLayoutTest, OffsetMovesTheWidgetInwardFromEachSide) {
    const QRect screen = primaryScreen();
    TaskbarReserve reserve;
    reserve.trailing = QRect(1600, 1040, 320, 40);

    EXPECT_EQ(taskbarWidgetX(screen, reserve, TaskbarSide::Right, 50, kWidth, kMargin), 1600 - kWidth - kMargin - 50);
    EXPECT_EQ(taskbarWidgetX(screen, TaskbarReserve{}, TaskbarSide::Left, 50, kWidth, kMargin), kMargin + 50);
    EXPECT_EQ(taskbarWidgetX(screen, TaskbarReserve{}, TaskbarSide::Left, -50, kWidth, kMargin), kMargin);
}

TEST(TaskbarLayoutTest, KeepsTheWidgetOnItsOwnScreen) {
    const QRect screen = secondaryScreen();
    TaskbarReserve reserve;
    reserve.trailing = QRect(1920, 1400, 2560, 40); // tray covers the whole taskbar

    const int wide = taskbarWidgetX(screen, reserve, TaskbarSide::Right, 0, kWidth, kMargin);
    EXPECT_GE(wide, screen.left());

    const int pushed = taskbarWidgetX(screen, TaskbarReserve{}, TaskbarSide::Left, 99999, kWidth, kMargin);
    EXPECT_LE(pushed + kWidth, screen.right() + 1);
}

// --- taskbar edge ----------------------------------------------------------

TEST(TaskbarLayoutTest, EdgeFollowsTheWorkAreaGap) {
    const QRect screen = primaryScreen();
    EXPECT_EQ(detectTaskbarEdge(screen, QRect(0, 0, 1920, 1032), QRect()), TaskbarEdge::Bottom);
    EXPECT_EQ(detectTaskbarEdge(screen, QRect(0, 48, 1920, 1032), QRect()), TaskbarEdge::Top);
    EXPECT_EQ(detectTaskbarEdge(screen, QRect(62, 0, 1858, 1080), QRect()), TaskbarEdge::Left);
    EXPECT_EQ(detectTaskbarEdge(screen, QRect(0, 0, 1858, 1080), QRect()), TaskbarEdge::Right);
}

// A docked side panel also reserves space; the taskbar is the larger gap.
TEST(TaskbarLayoutTest, LargestGapWins) {
    const QRect screen = primaryScreen();
    EXPECT_EQ(detectTaskbarEdge(screen, QRect(30, 0, 1890, 1032), QRect()), TaskbarEdge::Bottom);
    EXPECT_EQ(detectTaskbarEdge(screen, QRect(62, 0, 1858, 1050), QRect()), TaskbarEdge::Left);
}

TEST(TaskbarLayoutTest, AutoHideFallsBackToTheTaskbarWindow) {
    const QRect screen = primaryScreen();
    // Hidden taskbars leave a 2 px sliver on their edge; the window rect still tells the side.
    EXPECT_EQ(detectTaskbarEdge(screen, screen, QRect(0, 1078, 1920, 48)), TaskbarEdge::Bottom);
    EXPECT_EQ(detectTaskbarEdge(screen, screen, QRect(0, -46, 1920, 48)), TaskbarEdge::Top);
    EXPECT_EQ(detectTaskbarEdge(screen, screen, QRect(-60, 0, 62, 1080)), TaskbarEdge::Left);
    EXPECT_EQ(detectTaskbarEdge(screen, screen, QRect(1918, 0, 62, 1080)), TaskbarEdge::Right);
    EXPECT_EQ(detectTaskbarEdge(screen, screen, QRect()), TaskbarEdge::Bottom);
    // A window on another monitor says nothing about this one.
    EXPECT_EQ(detectTaskbarEdge(screen, screen, QRect(1920, 1392, 2560, 48)), TaskbarEdge::Bottom);
}

TEST(TaskbarLayoutTest, WidgetYCentresInTheTaskbarStripOnEitherEdge) {
    const QRect screen = primaryScreen();
    EXPECT_EQ(taskbarWidgetY(screen, QRect(0, 0, 1920, 1032), TaskbarEdge::Bottom, 32), 1032 + 8);
    EXPECT_EQ(taskbarWidgetY(screen, QRect(0, 48, 1920, 1032), TaskbarEdge::Top, 32), 8);
    // Taller than the strip: pinned to the strip's top edge, never above it.
    EXPECT_EQ(taskbarWidgetY(screen, QRect(0, 48, 1920, 1032), TaskbarEdge::Top, 60), 0);
    // Auto-hide: no gap, assume the default depth on the detected edge.
    EXPECT_EQ(taskbarWidgetY(screen, screen, TaskbarEdge::Top, 32), (kDefaultTaskbarDepth - 32) / 2);
    EXPECT_EQ(taskbarWidgetY(screen, screen, TaskbarEdge::Bottom, 32), 1080 - kDefaultTaskbarDepth + (kDefaultTaskbarDepth - 32) / 2);
}

TEST(TaskbarLayoutTest, OnlyHorizontalEdgesHoldAWidget) {
    EXPECT_TRUE(isHorizontalEdge(TaskbarEdge::Bottom));
    EXPECT_TRUE(isHorizontalEdge(TaskbarEdge::Top));
    EXPECT_FALSE(isHorizontalEdge(TaskbarEdge::Left));
    EXPECT_FALSE(isHorizontalEdge(TaskbarEdge::Right));
}
