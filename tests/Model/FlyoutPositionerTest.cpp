#include <gtest/gtest.h>

#include "FlyoutPositioner.h"

using namespace mf;

namespace {
const QRect kWork(0, 0, 1920, 1080);
const QSize kSize(360, 120);
constexpr int kMargin = 24;
} // namespace

TEST(FlyoutPositionerTest, PreservesRequestedSize) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::BottomRight, kMargin);
    EXPECT_EQ(r.size(), kSize);
}

TEST(FlyoutPositionerTest, TopLeft) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::TopLeft, kMargin);
    EXPECT_EQ(r.x(), kMargin);
    EXPECT_EQ(r.y(), kMargin);
}

TEST(FlyoutPositionerTest, TopCenter) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::TopCenter, kMargin);
    EXPECT_EQ(r.x(), (1920 - 360) / 2);
    EXPECT_EQ(r.y(), kMargin);
}

TEST(FlyoutPositionerTest, TopRight) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::TopRight, kMargin);
    EXPECT_EQ(r.x(), 1920 - 360 - kMargin);
    EXPECT_EQ(r.y(), kMargin);
}

TEST(FlyoutPositionerTest, BottomLeft) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::BottomLeft, kMargin);
    EXPECT_EQ(r.x(), kMargin);
    EXPECT_EQ(r.y(), 1080 - 120 - kMargin);
}

TEST(FlyoutPositionerTest, BottomCenter) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::BottomCenter, kMargin);
    EXPECT_EQ(r.x(), (1920 - 360) / 2);
    EXPECT_EQ(r.y(), 1080 - 120 - kMargin);
}

TEST(FlyoutPositionerTest, BottomRight) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::BottomRight, kMargin);
    EXPECT_EQ(r.x(), 1920 - 360 - kMargin);
    EXPECT_EQ(r.y(), 1080 - 120 - kMargin);
}

TEST(FlyoutPositionerTest, HonorsNonZeroWorkAreaOrigin) {
    const QRect work(100, 200, 1920, 1080);
    const QRect r = computeFlyoutRect(work, kSize, FlyoutPosition::TopLeft, kMargin);
    EXPECT_EQ(r.x(), 100 + kMargin);
    EXPECT_EQ(r.y(), 200 + kMargin);
}

TEST(FlyoutPositionerTest, ZeroMarginSitsFlushToEdges) {
    const QRect r = computeFlyoutRect(kWork, kSize, FlyoutPosition::BottomRight, 0);
    EXPECT_EQ(r.x(), 1920 - 360);
    EXPECT_EQ(r.y(), 1080 - 120);
}
