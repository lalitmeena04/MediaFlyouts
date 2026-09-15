#include <gtest/gtest.h>

#include "TaskbarLayout.h"

using mf::kUnmeasuredTrayWidth;
using mf::TaskbarReserve;
using mf::TaskbarSide;
using mf::taskbarWidgetX;

namespace {

constexpr int kMargin = 8;
constexpr int kWidth = 220;

// A second monitor placed to the right of a 1920-wide primary.
QRect secondaryScreen() {
    return QRect(1920, 0, 2560, 1440);
}

QRect primaryScreen() {
    return QRect(0, 0, 1920, 1080);
}

} // namespace

TEST(TaskbarLayoutTest, RightSideSitsBesideTheMeasuredTray) {
    TaskbarReserve reserve;
    reserve.trailing = QRect(1600, 1040, 320, 40);

    const int x = taskbarWidgetX(primaryScreen(), reserve, TaskbarSide::Right, 0, kWidth, kMargin);
    EXPECT_EQ(x, 1600 - kWidth - kMargin);
}

// The reported bug: a second monitor reserved the primary monitor's tray width.
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
    // A narrower tray must let the widget sit further right.
    EXPECT_GT(narrowX, wideX);
}

TEST(TaskbarLayoutTest, RightSideFallsBackWhenTheTrayIsNotMeasured) {
    const QRect screen = secondaryScreen();

    const int x = taskbarWidgetX(screen, TaskbarReserve{}, TaskbarSide::Right, 0, kWidth, kMargin);
    EXPECT_EQ(x, screen.right() + 1 - kUnmeasuredTrayWidth - kWidth - kMargin);
}

// Centred Windows 11 chrome is not an edge reserve. It must not move the widget.
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
    // Centre accepts a negative offset. The edges do not.
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
