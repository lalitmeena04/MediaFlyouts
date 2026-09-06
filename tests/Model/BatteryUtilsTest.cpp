#include "BatteryUtils.h"

#include <gtest/gtest.h>

using BatteryUtils::categoryFromClassOfDevice;
using BatteryUtils::cleanBluetoothProfileSuffix;
using BatteryUtils::milliwattHoursToPercent;

TEST(BatteryUtilsTest, ClassifiesAudioDevicesAsHeadsets) {
    EXPECT_EQ(categoryFromClassOfDevice(0x240404), "headset"); // major class 0x04, audio/video
}

TEST(BatteryUtilsTest, ClassifiesHidPeripherals) {
    EXPECT_EQ(categoryFromClassOfDevice(0x000540), "keyboard");
    EXPECT_EQ(categoryFromClassOfDevice(0x000580), "mouse");
    EXPECT_EQ(categoryFromClassOfDevice(0x0005C0), "keyboard"); // combo keyboard/pointer
    EXPECT_EQ(categoryFromClassOfDevice(0x000508), "gamepad");
}

TEST(BatteryUtilsTest, ReturnsEmptyForUnmappedClasses) {
    EXPECT_EQ(categoryFromClassOfDevice(0x000000), "");
    EXPECT_EQ(categoryFromClassOfDevice(0x000200), ""); // phone
}

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
    // Mid-name "Hands-Free" is not profile suffix.
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
    // Some firmware reports remaining above full capacity briefly.
    EXPECT_EQ(milliwattHoursToPercent(110, 100), 100);
}
