#include <gtest/gtest.h>

#include "WindowScalePolicy.h"

using namespace mf;

namespace {
WindowScaleSnapshot at(int w, int h, qreal dpr) {
    return WindowScaleSnapshot{QSize(w, h), dpr};
}
} // namespace

TEST(WindowScalePolicyTest, NativeSizeScalesAndRounds) {
    EXPECT_EQ(nativeSizeOf(QSize(186, 32), 1.5), QSize(279, 48));
    EXPECT_EQ(nativeSizeOf(QSize(186, 21), 1.5), QSize(279, 32)); // 31.5 rounds up
    EXPECT_EQ(nativeSizeOf(QSize(100, 50), 0.0), QSize(100, 50)); // zero dpr is 100 %
}

TEST(WindowScalePolicyTest, ResizeAtSameScaleIsAuthoritative) {
    EXPECT_TRUE(resizeIsAuthoritative(at(186, 32, 1.0), 1.0));
    EXPECT_TRUE(resizeIsAuthoritative(at(186, 32, 1.5), 1.5));
}

TEST(WindowScalePolicyTest, ResizeDuringScaleChangeIsNotAuthoritative) {
    EXPECT_FALSE(resizeIsAuthoritative(at(186, 32, 1.0), 1.5));
    EXPECT_FALSE(resizeIsAuthoritative(at(186, 32, 1.5), 1.0));
}

// The bug seen on hardware: scale 100 % -> 150 %, native stays 186x32,
// Qt reports 124x21 logical. Restore the logical size the app chose.
TEST(WindowScalePolicyTest, RestoresWhenNativeSizeDidNotFollowTheScale) {
    const QSize restore = sizeToRestore(at(186, 32, 1.0), QSize(186, 32), 1.5);
    EXPECT_EQ(restore, QSize(186, 32));
}

TEST(WindowScalePolicyTest, RestoresOnTheWayBackDownToo) {
    const QSize restore = sizeToRestore(at(244, 40, 1.5), QSize(366, 60), 1.0);
    EXPECT_EQ(restore, QSize(244, 40));
}

TEST(WindowScalePolicyTest, NoRestoreWhenQtAlreadyRescaled) {
    EXPECT_FALSE(sizeToRestore(at(186, 32, 1.0), QSize(279, 48), 1.5).isValid());
    // One pixel of rounding slack per axis.
    EXPECT_FALSE(sizeToRestore(at(186, 32, 1.0), QSize(280, 47), 1.5).isValid());
}

TEST(WindowScalePolicyTest, NoRestoreWhenScaleIsUnchanged) {
    EXPECT_FALSE(sizeToRestore(at(186, 32, 1.0), QSize(186, 32), 1.0).isValid());
    EXPECT_FALSE(sizeToRestore(at(186, 32, 1.5), QSize(279, 48), 1.5).isValid());
}

// A size that matches neither the old nor the new scale is a real resize
// the app made in between. Leave it alone.
TEST(WindowScalePolicyTest, NoRestoreForAnUnrelatedSize) {
    EXPECT_FALSE(sizeToRestore(at(186, 32, 1.0), QSize(500, 90), 1.5).isValid());
}

TEST(WindowScalePolicyTest, NoRestoreForInvalidInput) {
    EXPECT_FALSE(sizeToRestore(WindowScaleSnapshot{}, QSize(186, 32), 1.5).isValid());
    EXPECT_FALSE(sizeToRestore(at(186, 32, 1.0), QSize(), 1.5).isValid());
}
