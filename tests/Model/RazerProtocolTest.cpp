#include <gtest/gtest.h>

#include "RazerProtocol.h"

using namespace razer;

TEST(RazerProtocolTest, BuildsAWellFormedRequest) {
    const auto request = buildRequest(0x1F, kCommandBatteryLevel);

    ASSERT_EQ(request.size(), kBufferSize);
    EXPECT_EQ(request[0], 0x00);                                          // report id
    EXPECT_EQ(request[1], static_cast<std::uint8_t>(Status::NewCommand)); // status
    EXPECT_EQ(request[2], 0x1F);                                          // transaction id
    EXPECT_EQ(request[6], 0x02);                                          // data size
    EXPECT_EQ(request[7], kCommandClassPower);
    EXPECT_EQ(request[8], kCommandBatteryLevel);
    EXPECT_EQ(request[89], checksum(request));
}

TEST(RazerProtocolTest, ChecksumCoversTheCommandBytes) {
    auto request = buildRequest(0x1F, kCommandBatteryLevel);
    const std::uint8_t original = checksum(request);

    request[8] = kCommandChargingStatus; // changing the command must change the checksum
    EXPECT_NE(checksum(request), original);
}

TEST(RazerProtocolTest, ScalesTheRawByteFrom255To100) {
    // Captured from Basilisk V3 X HyperSpeed at 53%.
    EXPECT_EQ(percentFromRaw(135), 53);
    EXPECT_EQ(percentFromRaw(0), 0);
    EXPECT_EQ(percentFromRaw(255), 100);
    EXPECT_EQ(percentFromRaw(128), 50);
}

TEST(RazerProtocolTest, ReadsBatteryFromASuccessfulReply) {
    auto reply = buildRequest(0x2F, kCommandBatteryLevel);
    reply[1] = static_cast<std::uint8_t>(Status::Successful);
    reply[10] = 135; // arguments[1]

    EXPECT_EQ(batteryPercent(reply, 0x2F), 53);
}

TEST(RazerProtocolTest, IgnoresRepliesForAnotherTransaction) {
    auto reply = buildRequest(0x2F, kCommandBatteryLevel);
    reply[1] = static_cast<std::uint8_t>(Status::Successful);
    reply[10] = 135;

    EXPECT_EQ(batteryPercent(reply, 0x4F), -1);
}

TEST(RazerProtocolTest, TreatsBusyAndTimeoutAsNoAnswer) {
    auto busy = buildRequest(0x00, kCommandBatteryLevel);
    busy[1] = static_cast<std::uint8_t>(Status::Busy);
    busy[10] = 135;
    EXPECT_EQ(batteryPercent(busy, 0x00), -1);
    EXPECT_EQ(statusOf(busy), Status::Busy);

    auto timeout = buildRequest(0x9F, kCommandBatteryLevel);
    timeout[1] = static_cast<std::uint8_t>(Status::Timeout);
    EXPECT_EQ(batteryPercent(timeout, 0x9F), -1);
}

TEST(RazerProtocolTest, RejectsUnrelatedReports) {
    // Device also emits 0x0F/0x03 reports. They are not answers.
    auto other = buildRequest(0x1F, kCommandBatteryLevel);
    other[1] = static_cast<std::uint8_t>(Status::Successful);
    other[7] = 0x0F;
    other[8] = 0x03;

    EXPECT_FALSE(isReplyTo(other, 0x1F, kCommandBatteryLevel));
    EXPECT_EQ(batteryPercent(other, 0x1F), -1);
}

TEST(RazerProtocolTest, ReportsChargingSupportSeparatelyFromState) {
    auto notSupported = buildRequest(0x2F, kCommandChargingStatus);
    notSupported[1] = static_cast<std::uint8_t>(Status::NotSupported);
    bool supported = true;
    EXPECT_FALSE(chargingState(notSupported, 0x2F, supported));
    EXPECT_FALSE(supported); // Several wireless models lack 0x84.

    auto charging = buildRequest(0x2F, kCommandChargingStatus);
    charging[1] = static_cast<std::uint8_t>(Status::Successful);
    charging[10] = 0x01;
    EXPECT_TRUE(chargingState(charging, 0x2F, supported));
    EXPECT_TRUE(supported);
}
