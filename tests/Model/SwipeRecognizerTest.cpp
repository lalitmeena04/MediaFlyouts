#include <gtest/gtest.h>

#include "SwipeRecognizer.h"

using namespace mf;

TEST(SwipeRecognizerTest, ClearSwipesMapToTheFourDirections) {
    EXPECT_EQ(recognizeSwipe(QPointF(-80, 5), 200), SwipeDirection::Left);
    EXPECT_EQ(recognizeSwipe(QPointF(80, -5), 200), SwipeDirection::Right);
    EXPECT_EQ(recognizeSwipe(QPointF(3, -80), 200), SwipeDirection::Up);
    EXPECT_EQ(recognizeSwipe(QPointF(-3, 80), 200), SwipeDirection::Down);
}

TEST(SwipeRecognizerTest, ShortTravelIsNotASwipe) {
    EXPECT_EQ(recognizeSwipe(QPointF(-47, 0), 100), SwipeDirection::None);
    EXPECT_EQ(recognizeSwipe(QPointF(0, 47), 100), SwipeDirection::None);
    EXPECT_EQ(recognizeSwipe(QPointF(48, 0), 100), SwipeDirection::Right);
}

TEST(SwipeRecognizerTest, SlowDragIsNotASwipe) {
    EXPECT_EQ(recognizeSwipe(QPointF(-120, 0), 601), SwipeDirection::None);
    EXPECT_EQ(recognizeSwipe(QPointF(-120, 0), 600), SwipeDirection::Left);
    EXPECT_EQ(recognizeSwipe(QPointF(-120, 0), -1), SwipeDirection::None);
}

TEST(SwipeRecognizerTest, DiagonalIsNotASwipe) {
    EXPECT_EQ(recognizeSwipe(QPointF(100, 70), 200), SwipeDirection::None);
    EXPECT_EQ(recognizeSwipe(QPointF(100, 55), 200), SwipeDirection::Right);
}

TEST(SwipeRecognizerTest, LargerAxisDecidesDirection) {
    EXPECT_EQ(recognizeSwipe(QPointF(60, 100), 200), SwipeDirection::Down);
    EXPECT_EQ(recognizeSwipe(QPointF(-100, 60), 200), SwipeDirection::Left);
}

TEST(SwipeRecognizerTest, ThresholdsAreTunable) {
    SwipeThresholds loose;
    loose.minDistance = 10;
    loose.maxDurationMs = 5000;
    loose.maxCrossRatio = 1.0;
    EXPECT_EQ(recognizeSwipe(QPointF(12, 11), 4000, loose), SwipeDirection::Right);
}

TEST(SwipeRecognizerTest, AxisLocksAfterSmallTravelAndStaysLocked) {
    EXPECT_EQ(lockSwipeAxis(QPointF(3, 2), SwipeAxis::None), SwipeAxis::None);
    EXPECT_EQ(lockSwipeAxis(QPointF(9, 2), SwipeAxis::None), SwipeAxis::Horizontal);
    EXPECT_EQ(lockSwipeAxis(QPointF(2, -9), SwipeAxis::None), SwipeAxis::Vertical);
    EXPECT_EQ(lockSwipeAxis(QPointF(5, 200), SwipeAxis::Horizontal), SwipeAxis::Horizontal);
    EXPECT_EQ(lockSwipeAxis(QPointF(200, 5), SwipeAxis::Vertical), SwipeAxis::Vertical);
}

TEST(SwipeRecognizerTest, ProjectionKeepsOnlyTheLockedAxis) {
    EXPECT_EQ(projectOnAxis(QPointF(40, 30), SwipeAxis::Horizontal), QPointF(40, 0));
    EXPECT_EQ(projectOnAxis(QPointF(40, 30), SwipeAxis::Vertical), QPointF(0, 30));
    EXPECT_EQ(projectOnAxis(QPointF(40, 30), SwipeAxis::None), QPointF(0, 0));
}

TEST(SwipeRecognizerTest, VectorsAreUnitAxes) {
    EXPECT_EQ(swipeVector(SwipeDirection::Left), QPointF(-1, 0));
    EXPECT_EQ(swipeVector(SwipeDirection::Right), QPointF(1, 0));
    EXPECT_EQ(swipeVector(SwipeDirection::Up), QPointF(0, -1));
    EXPECT_EQ(swipeVector(SwipeDirection::Down), QPointF(0, 1));
    EXPECT_EQ(swipeVector(SwipeDirection::None), QPointF(0, 0));
}

TEST(SwipeRecognizerTest, EnumValuesMatchTheQmlContract) {
    EXPECT_EQ(static_cast<int>(SwipeDirection::None), 0);
    EXPECT_EQ(static_cast<int>(SwipeDirection::Left), 1);
    EXPECT_EQ(static_cast<int>(SwipeDirection::Right), 2);
    EXPECT_EQ(static_cast<int>(SwipeDirection::Up), 3);
    EXPECT_EQ(static_cast<int>(SwipeDirection::Down), 4);
    EXPECT_EQ(static_cast<int>(SwipeAxis::None), 0);
    EXPECT_EQ(static_cast<int>(SwipeAxis::Horizontal), 1);
    EXPECT_EQ(static_cast<int>(SwipeAxis::Vertical), 2);
}
