#include <gtest/gtest.h>

#include <QSignalSpy>

#include "MonitorMapper.h"

TEST(MonitorMapperTest, CoalescesBurstOfRefreshesIntoOneChange) {
    MonitorMapper map(nullptr, 20);
    QSignalSpy changed(&map, &MonitorMapper::changed);

    map.refresh();
    map.refresh();
    map.refresh();
    EXPECT_EQ(changed.count(), 0) << "changed() must wait for the coalesce window";

    ASSERT_TRUE(changed.wait(500));
    EXPECT_EQ(changed.count(), 1);
}

TEST(MonitorMapperTest, NotifiesAgainForALaterBurst) {
    MonitorMapper map(nullptr, 10);
    QSignalSpy changed(&map, &MonitorMapper::changed);

    map.refresh();
    ASSERT_TRUE(changed.wait(500));
    map.refresh();
    ASSERT_TRUE(changed.wait(500));

    EXPECT_EQ(changed.count(), 2);
}

TEST(MonitorMapperTest, EmptyHostYieldsNoMonitors) {
    MonitorMapper map;
    EXPECT_EQ(map.count(), 0);
    EXPECT_TRUE(map.names().isEmpty());
    EXPECT_EQ(map.entry(0), nullptr);
    EXPECT_EQ(map.entry(-1), nullptr);
    EXPECT_TRUE(map.geometry(0).isEmpty());
    EXPECT_TRUE(map.availableGeometry(3).isEmpty());
}

TEST(MonitorMapperTest, OutOfRangeIndexFallsBackToZero) {
    MonitorMapper map;
    EXPECT_EQ(map.clampIndex(-1), 0);
    EXPECT_EQ(map.clampIndex(0), 0);
    EXPECT_EQ(map.clampIndex(7), 0);
}

TEST(MonitorMapperTest, ToLogicalUsesTheMonitorsOwnScaleAndOrigin) {
    MonitorEntry monitor;
    monitor.logicalRect = QRect(2560, 0, 1707, 960);
    monitor.nativeRect = QRect(2560, 0, 2560, 1440);
    monitor.devicePixelRatio = 1.5;

    const QRect nativeTray(2560 + 2560 - 300, 1440 - 72, 300, 72);
    const QRect logical = MonitorMapper::toLogical(monitor, nativeTray);

    EXPECT_EQ(logical, QRect(2560 + 1707 - 200, 960 - 48, 200, 48));
}

TEST(MonitorMapperTest, ToLogicalIsIdentityAtOneHundredPercent) {
    MonitorEntry monitor;
    monitor.logicalRect = QRect(0, 0, 1920, 1080);
    monitor.nativeRect = monitor.logicalRect;
    monitor.devicePixelRatio = 1.0;

    const QRect rect(1620, 1032, 300, 48);
    EXPECT_EQ(MonitorMapper::toLogical(monitor, rect), rect);
}

TEST(MonitorMapperTest, ToLogicalHandlesNegativeOrigins) {
    MonitorEntry monitor;
    monitor.logicalRect = QRect(-1920, -1080, 960, 540);
    monitor.nativeRect = QRect(-1920, -1080, 1920, 1080);
    monitor.devicePixelRatio = 2.0;

    const QRect nativeStart(-1920, -1080 + 1080 - 80, 120, 80);
    EXPECT_EQ(MonitorMapper::toLogical(monitor, nativeStart), QRect(-1920, -1080 + 540 - 40, 60, 40));
}

TEST(MonitorMapperTest, ToLogicalMapsNullRectToNull) {
    MonitorEntry monitor;
    monitor.logicalRect = QRect(0, 0, 1920, 1080);
    monitor.nativeRect = monitor.logicalRect;
    EXPECT_TRUE(MonitorMapper::toLogical(monitor, QRect()).isNull());
}

TEST(MonitorMapperTest, ToLogicalTreatsZeroRatioAsOne) {
    MonitorEntry monitor;
    monitor.logicalRect = QRect(0, 0, 100, 100);
    monitor.nativeRect = monitor.logicalRect;
    monitor.devicePixelRatio = 0.0;

    const QRect rect(10, 20, 30, 40);
    EXPECT_EQ(MonitorMapper::toLogical(monitor, rect), rect);
}
