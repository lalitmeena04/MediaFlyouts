#include "DualSenseProtocol.h"

#include <gtest/gtest.h>

using DualSenseProtocol::buildEnableExtendedReportPacket;
using DualSenseProtocol::crc32;
using DualSenseProtocol::parseBatteryPercent;

TEST(DualSenseProtocolTest, Crc32MatchesStandardCheckValue) {
    const std::string testVector("123456789");
    const std::vector<uint8_t> input(testVector.begin(), testVector.end());
    EXPECT_EQ(crc32(input), 0xCBF43926u);
}

TEST(DualSenseProtocolTest, Crc32OfEmptyInputIsZero) {
    EXPECT_EQ(crc32({}), 0u);
}

TEST(DualSenseProtocolTest, EnableExtendedReportPacketHasExpectedHeaderAndLength) {
    const auto packet = buildEnableExtendedReportPacket();
    ASSERT_EQ(packet.size(), 78u);
    EXPECT_EQ(packet[0], 0x31);
    EXPECT_EQ(packet[1], 0x02);
    EXPECT_EQ(packet[2], 0x15);
}

TEST(DualSenseProtocolTest, EnableExtendedReportPacketCrcIsInternallyConsistent) {
    const auto packet = buildEnableExtendedReportPacket();
    ASSERT_EQ(packet.size(), 78u);

    std::vector<uint8_t> crcInput;
    crcInput.push_back(0xA2);
    crcInput.insert(crcInput.end(), packet.begin(), packet.begin() + 74);
    const uint32_t expectedCrc = crc32(crcInput);

    const uint32_t storedCrc = static_cast<uint32_t>(packet[74]) | (static_cast<uint32_t>(packet[75]) << 8)
        | (static_cast<uint32_t>(packet[76]) << 16) | (static_cast<uint32_t>(packet[77]) << 24);
    EXPECT_EQ(storedCrc, expectedCrc);
}

TEST(DualSenseProtocolTest, ParsesUsbBatteryFromLowNibble) {
    std::vector<uint8_t> report(64, 0);
    report[53] = 0x04; // 4/8 -> 50%
    EXPECT_EQ(parseBatteryPercent(report, /*isBluetooth=*/false), 50);
}

TEST(DualSenseProtocolTest, FullChargeFlagReportsHundredPercent) {
    std::vector<uint8_t> report(64, 0);
    report[53] = 0x20; // full-charge bit set, nibble ignored
    EXPECT_EQ(parseBatteryPercent(report, /*isBluetooth=*/false), 100);
}

TEST(DualSenseProtocolTest, BluetoothUsesOneByteOffsetAndRequiresExtendedReportId) {
    std::vector<uint8_t> report(78, 0);
    report[0] = 0x31; // extended report id
    report[54] = 0x08; // 8/8 -> 100%
    EXPECT_EQ(parseBatteryPercent(report, /*isBluetooth=*/true), 100);
}

TEST(DualSenseProtocolTest, BluetoothSimpleReportHasNoBatteryData) {
    std::vector<uint8_t> report(78, 0);
    report[0] = 0x01; // still on the basic report, not yet switched to extended mode
    report[54] = 0x08;
    EXPECT_EQ(parseBatteryPercent(report, /*isBluetooth=*/true), -1);
}

TEST(DualSenseProtocolTest, TooShortReportReturnsInvalid) {
    std::vector<uint8_t> report(10, 0);
    EXPECT_EQ(parseBatteryPercent(report, /*isBluetooth=*/false), -1);
    EXPECT_EQ(parseBatteryPercent(report, /*isBluetooth=*/true), -1);
}
