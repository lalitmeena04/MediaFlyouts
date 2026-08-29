#include <gtest/gtest.h>

#include "HidppProtocol.h"

using namespace hidpp;

TEST(HidppProtocolTest, BuildsAGetFeatureIndexRequest) {
    const auto report = buildGetFeatureIndex(kDirectDeviceIndex, kFeatureUnifiedBattery);

    ASSERT_EQ(report.size(), kShortReportSize);
    EXPECT_EQ(report[0], kShortReportId);
    EXPECT_EQ(report[1], kDirectDeviceIndex);
    EXPECT_EQ(report[2], 0x00);         // root feature
    EXPECT_EQ(report[3], kSoftwareId);  // function 0 in the high nibble
    EXPECT_EQ(report[4], 0x10);         // 0x1004 split across two bytes
    EXPECT_EQ(report[5], 0x04);
}

TEST(HidppProtocolTest, PacksFunctionAndSoftwareIdIntoOneByte) {
    const auto report = buildFunctionCall(0x01, 0x06, 0x01);

    EXPECT_EQ(report[1], 0x01);
    EXPECT_EQ(report[2], 0x06);
    EXPECT_EQ(report[3], 0x1D); // function 1 << 4 | software id 0x0D
}

TEST(HidppProtocolTest, AcceptsRepliesCarryingOurSoftwareId) {
    std::vector<std::uint8_t> reply{kShortReportId, 0xFF, 0x06, kSoftwareId, 0x00, 0x00, 0x00};
    EXPECT_TRUE(isReplyFor(reply, 0xFF));

    reply[3] = 0x1D; // a function reply keeps the software id in the low nibble
    EXPECT_TRUE(isReplyFor(reply, 0xFF));
}

TEST(HidppProtocolTest, RejectsNotificationsAndOtherDevices) {
    // a notification from the device uses a different software id
    std::vector<std::uint8_t> notification{kShortReportId, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00};
    EXPECT_FALSE(isReplyFor(notification, 0xFF));

    std::vector<std::uint8_t> otherDevice{kShortReportId, 0x02, 0x06, kSoftwareId, 0x00, 0x00, 0x00};
    EXPECT_FALSE(isReplyFor(otherDevice, 0x01));
}

TEST(HidppProtocolTest, DetectsErrorReports) {
    std::vector<std::uint8_t> error{kShortReportId, 0xFF, 0xFF, kSoftwareId, 0x01, 0x00, 0x00};
    EXPECT_TRUE(isError(error));
    EXPECT_EQ(featureIndexOf(error), 0);
}

TEST(HidppProtocolTest, ReadsTheFeatureIndex) {
    std::vector<std::uint8_t> reply{kShortReportId, 0xFF, 0x00, kSoftwareId, 0x06, 0x00, 0x00};
    EXPECT_EQ(featureIndexOf(reply), 0x06);

    // index 0 means the device doesn't implement the feature
    std::vector<std::uint8_t> unsupported{kShortReportId, 0xFF, 0x00, kSoftwareId, 0x00, 0x00, 0x00};
    EXPECT_EQ(featureIndexOf(unsupported), 0);
}

TEST(HidppProtocolTest, ReadsBatteryPercentages) {
    std::vector<std::uint8_t> unified{kShortReportId, 0xFF, 0x06, 0x1D, 75, 0x00, 0x00};
    EXPECT_EQ(unifiedBatteryPercent(unified), 75);

    std::vector<std::uint8_t> legacy{kShortReportId, 0xFF, 0x05, 0x0D, 40, 0x00, 0x00};
    EXPECT_EQ(legacyBatteryPercent(legacy), 40);
}

TEST(HidppProtocolTest, RejectsOutOfRangeAndErrorBatteryReplies) {
    std::vector<std::uint8_t> tooHigh{kShortReportId, 0xFF, 0x06, 0x1D, 200, 0x00, 0x00};
    EXPECT_EQ(unifiedBatteryPercent(tooHigh), -1);

    std::vector<std::uint8_t> error{kShortReportId, 0xFF, 0xFF, 0x1D, 0x01, 0x00, 0x00};
    EXPECT_EQ(unifiedBatteryPercent(error), -1);
}
