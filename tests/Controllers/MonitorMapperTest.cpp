#include <gtest/gtest.h>

#include <QGuiApplication>
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

TEST(MonitorMapperTest, MirrorsTheHostScreenList) {
    MonitorMapper map;
    const int screens = QGuiApplication::screens().size();
    EXPECT_EQ(map.count(), screens);
    EXPECT_EQ(map.names().size(), screens);
    for(int i = 0; i < screens; ++i) {
        ASSERT_NE(map.entry(i), nullptr) << "monitor " << i;
        EXPECT_FALSE(map.geometry(i).isEmpty()) << "monitor " << i;
        EXPECT_FALSE(map.availableGeometry(i).isEmpty()) << "monitor " << i;
    }
}

TEST(MonitorMapperTest, OutOfRangeIndexFallsBackToTheFirstMonitor) {
    MonitorMapper map;
    const int screens = QGuiApplication::screens().size();
    // Without screens nothing can be returned. With screens, index 0 is the fallback.
    const MonitorEntry* fallback = screens > 0 ? map.entry(0) : nullptr;
    EXPECT_EQ(map.entry(-1), fallback);
    EXPECT_EQ(map.entry(screens), fallback);
    EXPECT_EQ(map.geometry(screens), map.geometry(0));
    EXPECT_EQ(map.availableGeometry(screens + 3), map.availableGeometry(0));
}

TEST(MonitorMapperTest, OutOfRangeIndexFallsBackToZero) {
    MonitorMapper map;
    const int screens = QGuiApplication::screens().size();
    EXPECT_EQ(map.clampIndex(-1), 0);
    EXPECT_EQ(map.clampIndex(0), 0);
    EXPECT_EQ(map.clampIndex(screens + 7), 0);
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
