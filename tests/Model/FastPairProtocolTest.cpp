#include <gtest/gtest.h>

#include "FastPairProtocol.h"

using namespace fastpair;

namespace {
std::vector<std::uint8_t> bytes(std::initializer_list<int> values) {
    return std::vector<std::uint8_t>(values.begin(), values.end());
}
} // namespace

TEST(FastPairProtocolTest, DrainsCompleteFramesAndKeepsPartialTail) {
    auto buffer = bytes({0x03, 0x01, 0x00, 0x03, 0x3F, 0x6F, 0x4A, 0x03, 0x02, 0x00});
    const auto frames = drainFrames(buffer);

    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].group, 0x03);
    EXPECT_EQ(frames[0].code, 0x01);
    EXPECT_EQ(frames[0].payload, bytes({0x3F, 0x6F, 0x4A}));
    EXPECT_EQ(buffer.size(), 3u);
}

TEST(FastPairProtocolTest, DrainsSeveralFramesFromOneRead) {
    auto buffer = bytes({0x01, 0x02, 0x00, 0x00, 0x03, 0x03, 0x00, 0x01, 0x46});
    const auto frames = drainFrames(buffer);

    ASSERT_EQ(frames.size(), 2u);
    EXPECT_FALSE(isBatteryFrame(frames[0]));
    EXPECT_TRUE(isBatteryFrame(frames[1]));
    EXPECT_TRUE(buffer.empty());
}

TEST(FastPairProtocolTest, ParsesSingleComponentDevice) {
    // Captured from CMF Neckband Pro. 70%.
    auto buffer = bytes({0x03, 0x03, 0x00, 0x01, 0x46});
    const auto frames = drainFrames(buffer);
    ASSERT_EQ(frames.size(), 1u);

    const auto components = parseBattery(frames[0]);
    ASSERT_EQ(components.size(), 1u);
    EXPECT_EQ(components[0].percent, 70);
    EXPECT_FALSE(components[0].charging);
    EXPECT_EQ(lowestPercent(components), 70);
    EXPECT_FALSE(anyCharging(components));
}

TEST(FastPairProtocolTest, ParsesTrueWirelessLeftRightCase) {
    // Captured from Nothing Ear (2). Left 100%. Right 90%. Case 100%.
    auto buffer = bytes({0x03, 0x03, 0x00, 0x03, 0x64, 0x5A, 0x64});
    const auto frames = drainFrames(buffer);
    ASSERT_EQ(frames.size(), 1u);

    const auto components = parseBattery(frames[0]);
    ASSERT_EQ(components.size(), 3u);
    EXPECT_EQ(components[0].percent, 100);
    EXPECT_EQ(components[1].percent, 90);
    EXPECT_EQ(components[2].percent, 100);
    EXPECT_EQ(lowestPercent(components), 90);
}

TEST(FastPairProtocolTest, DecodesTheChargingBit) {
    // Same device. Case on charger. 0xE4 = 0x80 | 100.
    auto buffer = bytes({0x03, 0x03, 0x00, 0x03, 0x64, 0x50, 0xE4});
    const auto frames = drainFrames(buffer);
    const auto components = parseBattery(frames[0]);

    ASSERT_EQ(components.size(), 3u);
    EXPECT_EQ(components[2].percent, 100);
    EXPECT_TRUE(components[2].charging);
    EXPECT_FALSE(components[0].charging);
    EXPECT_EQ(lowestPercent(components), 80);
    EXPECT_TRUE(anyCharging(components));
}

TEST(FastPairProtocolTest, TreatsUnknownLevelsAsMissing) {
    auto buffer = bytes({0x03, 0x03, 0x00, 0x03, 0xFF, 0x5A, 0xFF});
    const auto frames = drainFrames(buffer);
    const auto components = parseBattery(frames[0]);

    ASSERT_EQ(components.size(), 3u);
    EXPECT_EQ(components[0].percent, -1);
    EXPECT_EQ(components[1].percent, 90);
    EXPECT_EQ(lowestPercent(components), 90);
}

TEST(FastPairProtocolTest, IgnoresNonBatteryFrames) {
    auto buffer = bytes({0x03, 0x09, 0x00, 0x03, 0x31, 0x2E, 0x30});
    const auto frames = drainFrames(buffer);

    ASSERT_EQ(frames.size(), 1u);
    EXPECT_FALSE(isBatteryFrame(frames[0]));
    EXPECT_TRUE(parseBattery(frames[0]).empty());
}

TEST(FastPairProtocolTest, LowestPercentIsUnknownWhenNothingIsKnown) {
    std::vector<BatteryComponent> components{{-1, false}, {-1, true}};
    EXPECT_EQ(lowestPercent(components), -1);
    EXPECT_TRUE(anyCharging(components));
}
