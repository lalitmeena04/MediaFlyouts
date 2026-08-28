#include "BatteryUtils.h"

#include <gtest/gtest.h>

using BatteryUtils::cleanBluetoothProfileSuffix;
using BatteryUtils::milliwattHoursToPercent;

TEST(BatteryUtilsTest, StripsHandsFreeAgSuffix) {
    EXPECT_EQ(cleanBluetoothProfileSuffix("CMF Neckband Pro Hands-Free AG"), "CMF Neckband Pro");
}

TEST(BatteryUtilsTest, StripsAvrcpTransportSuffix) {
    EXPECT_EQ(cleanBluetoothProfileSuffix("Stone 358 Pro Avrcp Transport"), "Stone 358 Pro");
}

TEST(BatteryUtilsTest, StripIsCaseInsensitive) {
    EXPECT_EQ(cleanBluetoothProfileSuffix("Foo hands-free ag"), "Foo");
}

TEST(BatteryUtilsTest, LeavesPlainNameUnchanged) {
    EXPECT_EQ(cleanBluetoothProfileSuffix("BSK V3 X HS"), "BSK V3 X HS");
}

TEST(BatteryUtilsTest, OnlyStripsAtEndOfString) {
    // "Hands-Free" appearing mid-name (not a trailing profile suffix) must survive
    EXPECT_EQ(cleanBluetoothProfileSuffix("Hands-Free Speaker"), "Hands-Free Speaker");
}

TEST(BatteryUtilsTest, MilliwattHoursConvertsToPercent) {
    EXPECT_EQ(milliwattHoursToPercent(50, 100), 50);
    EXPECT_EQ(milliwattHoursToPercent(100, 100), 100);
    EXPECT_EQ(milliwattHoursToPercent(0, 100), 0);
}

TEST(BatteryUtilsTest, MilliwattHoursRejectsInvalidCapacity) {
    EXPECT_EQ(milliwattHoursToPercent(50, 0), -1);
    EXPECT_EQ(milliwattHoursToPercent(50, -10), -1);
    EXPECT_EQ(milliwattHoursToPercent(-5, 100), -1);
}

TEST(BatteryUtilsTest, MilliwattHoursClampsOverfullReading) {
    // some firmwares briefly report remaining slightly above the "full" capacity
    EXPECT_EQ(milliwattHoursToPercent(110, 100), 100);
}
