#include <gtest/gtest.h>

#include "JabraGnpProtocol.h"

using namespace gnp;

namespace {

// Response frame as the dongle sends it: 05 <dst=host> <src> <seq> <3<<6 | len> <cmd> <sub> <payload>.
std::vector<std::uint8_t> response(std::uint8_t source, std::uint8_t sequence, std::uint8_t command,
                                   std::uint8_t subCommand, std::vector<std::uint8_t> payload) {
    std::vector<std::uint8_t> frame(kFrameSize, 0);
    frame[0] = kReportId;
    frame[1] = kAddressHost;
    frame[2] = source;
    frame[3] = sequence;
    frame[4] = static_cast<std::uint8_t>((0x03 << 6) | (6 + payload.size()));
    frame[5] = command;
    frame[6] = subCommand;
    std::copy(payload.begin(), payload.end(), frame.begin() + 7);
    return frame;
}

std::vector<std::uint8_t> nack(std::uint8_t sequence, std::uint8_t reason) {
    return response(kAddressDongle, sequence, 0xFE, reason, {});
}

} // namespace

TEST(JabraGnpProtocolTest, MapsVendorCollectionUsageToHeadsetAddress) {
    EXPECT_EQ(headsetAddressForUsage(kVendorUsagePage, kDongleUsage), kAddressHeadsetViaDongle);
    EXPECT_EQ(headsetAddressForUsage(kVendorUsagePage, kHeadsetUsage), kAddressHeadsetOnUsb);
    EXPECT_EQ(headsetAddressForUsage(kVendorUsagePage, 0x02), 0);
    EXPECT_EQ(headsetAddressForUsage(0xFF30, kDongleUsage), 0); // other Jabra vendor page
    EXPECT_EQ(headsetAddressForUsage(0x000B, 0x05), 0);         // telephony headset collection
}

TEST(JabraGnpProtocolTest, BuildsAGetFrameForTheHeadsetBehindTheDongle) {
    const auto frame = buildGet(kAddressHeadsetViaDongle, 0x2A, kCommandStatus, kStatusHeadsetBattery);

    ASSERT_EQ(frame.size(), kFrameSize);
    EXPECT_EQ(frame[0], kReportId);
    EXPECT_EQ(frame[1], kAddressHeadsetViaDongle);
    EXPECT_EQ(frame[2], kAddressHost);
    EXPECT_EQ(frame[3], 0x2A);
    EXPECT_EQ(frame[4], 0x46); // GET (1 << 6) with length 6: no payload
    EXPECT_EQ(frame[5], kCommandStatus);
    EXPECT_EQ(frame[6], kStatusHeadsetBattery);
    for(std::size_t i = kHeaderSize; i < frame.size(); ++i)
        EXPECT_EQ(frame[i], 0) << "byte " << i;
}

TEST(JabraGnpProtocolTest, MatchesRepliesBySequence) {
    const auto request = buildGet(kAddressHeadsetViaDongle, 0x07, kCommandStatus, kStatusHeadsetBattery);

    EXPECT_TRUE(isReplyTo(response(kAddressHeadsetViaDongle, 0x07, kCommandStatus, kStatusHeadsetBattery, {0, 80}),
                          request));
    EXPECT_FALSE(isReplyTo(response(kAddressHeadsetViaDongle, 0x08, kCommandStatus, kStatusHeadsetBattery, {0, 80}),
                           request));
    EXPECT_TRUE(isReplyTo(nack(0x07, 0xF3), request));
}

TEST(JabraGnpProtocolTest, IgnoresEventsAndForeignFrames) {
    const auto request = buildGet(kAddressHeadsetViaDongle, 0x07, kCommandStatus, kStatusHeadsetBattery);

    std::vector<std::uint8_t> event = response(kAddressHeadsetViaDongle, 0x07, kCommandStatus, 0x08, {0x01});
    event[4] = 0x07; // type 0: unsolicited event with the same sequence byte
    EXPECT_FALSE(isReplyTo(event, request));

    EXPECT_FALSE(isReplyTo({0x01, 0x00, 0x04, 0x07, 0xC8, 0x12, 0x02, 0x00, 0x50}, request)); // other report id
    EXPECT_FALSE(isReplyTo({kReportId, 0x00, 0x04}, request));                                 // too short
    EXPECT_FALSE(isReplyTo(response(kAddressHeadsetViaDongle, 0x07, kCommandStatus, 0x02, {}), {})); // no request
}

TEST(JabraGnpProtocolTest, TellsOfflineNackFromUnsupportedCommand) {
    EXPECT_TRUE(isNack(nack(1, 0xF3)));
    EXPECT_TRUE(nackMeansOffline(nack(1, 0xF3))); // peer offline
    EXPECT_TRUE(nackMeansOffline(nack(1, 0xF4))); // no child
    EXPECT_TRUE(nackMeansOffline(nack(1, 0xF5))); // no device at that address
    EXPECT_FALSE(nackMeansOffline(nack(1, 0xF6))); // unknown command group
    EXPECT_FALSE(nackMeansOffline(nack(1, 0xFF))); // unknown sub-command

    const auto ok = response(kAddressHeadsetViaDongle, 1, kCommandStatus, kStatusHeadsetBattery, {0, 50});
    EXPECT_FALSE(isNack(ok));
    EXPECT_FALSE(nackMeansOffline(ok));
}

TEST(JabraGnpProtocolTest, ExtractsPayloadByDeclaredLength) {
    const auto frame = response(kAddressHeadsetViaDongle, 1, kCommandStatus, kStatusHeadsetBattery,
                                {0x01, 0x55, 0x0F, 0xA0});

    EXPECT_EQ(payloadOf(frame), (std::vector<std::uint8_t>{0x01, 0x55, 0x0F, 0xA0}));
    EXPECT_TRUE(payloadOf(nack(1, 0xF3)).empty());
    EXPECT_TRUE(payloadOf(response(kAddressHeadsetViaDongle, 1, 0xFF, 0x00, {})).empty()); // bare ACK
}

TEST(JabraGnpProtocolTest, ClampsPayloadToFrameSize) {
    std::vector<std::uint8_t> frame = response(kAddressHeadsetViaDongle, 1, kCommandIdentity, kIdentityName, {});
    frame[4] = static_cast<std::uint8_t>((0x03 << 6) | 0x3F); // claims 57 payload bytes
    frame.resize(kHeaderSize + 4);
    frame[7] = 0x03;
    frame[8] = 'A';
    frame[9] = 'B';
    frame[10] = 'C';

    EXPECT_EQ(payloadOf(frame).size(), 4u);
    EXPECT_EQ(identityString(frame), "ABC");
}

TEST(JabraGnpProtocolTest, DecodesBatteryLevelAndChargingFlag) {
    const auto discharging = response(kAddressHeadsetViaDongle, 1, kCommandStatus, kStatusHeadsetBattery,
                                      {0x00, 0x47, 0x0F, 0xA0});
    EXPECT_EQ(batteryPercent(discharging), 71);
    EXPECT_FALSE(batteryCharging(discharging));

    const auto charging = response(kAddressHeadsetViaDongle, 1, kCommandStatus, kStatusHeadsetBattery,
                                   {0x41, 0x64, 0x10, 0x00}); // charging + fast charging
    EXPECT_EQ(batteryPercent(charging), 100);
    EXPECT_TRUE(batteryCharging(charging));

    const auto low = response(kAddressHeadsetViaDongle, 1, kCommandStatus, kStatusHeadsetBattery, {0x02, 0x05});
    EXPECT_EQ(batteryPercent(low), 5);
    EXPECT_FALSE(batteryCharging(low));
}

TEST(JabraGnpProtocolTest, RejectsUnusableBatteryReplies) {
    EXPECT_EQ(batteryPercent(nack(1, 0xF3)), -1);
    EXPECT_FALSE(batteryCharging(nack(1, 0xF3)));
    EXPECT_EQ(batteryPercent(response(kAddressHeadsetViaDongle, 1, kCommandStatus, kStatusHeadsetBattery, {0x01})),
              -1);
    EXPECT_EQ(batteryPercent(response(kAddressHeadsetViaDongle, 1, kCommandStatus, kStatusHeadsetBattery,
                                      {0x00, 0xFF})),
              100); // out of range is clamped, not dropped
    EXPECT_EQ(batteryPercent({}), -1);
}

TEST(JabraGnpProtocolTest, DecodesLengthPrefixedIdentityString) {
    const std::string name = "Jabra Evolve2 65";
    std::vector<std::uint8_t> payload{static_cast<std::uint8_t>(name.size())};
    payload.insert(payload.end(), name.begin(), name.end());

    EXPECT_EQ(identityString(response(kAddressHeadsetViaDongle, 1, kCommandIdentity, kIdentityName, payload)),
              name);
}

TEST(JabraGnpProtocolTest, DecodesIdentityStringWithoutPrefixAndStopsAtNul) {
    const std::vector<std::uint8_t> payload{'L', 'i', 'n', 'k', 0x00, 'x', 'y'};

    EXPECT_EQ(identityString(response(kAddressDongle, 1, kCommandIdentity, kIdentityName, payload)), "Link");
    EXPECT_TRUE(identityString(nack(1, 0xF5)).empty());
    EXPECT_TRUE(identityString(response(kAddressDongle, 1, kCommandIdentity, kIdentityName, {})).empty());
}
