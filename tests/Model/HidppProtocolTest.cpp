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

TEST(HidppProtocolTest, CopiesParametersIntoFunctionCall) {
    const auto report = buildFunctionCall(0x02, 0x03, 0x01, {0x10});

    ASSERT_EQ(report.size(), kShortReportSize);
    EXPECT_EQ(report[3], 0x1D);
    EXPECT_EQ(report[4], 0x10);
}

TEST(HidppProtocolTest, ConvertsShortRequestToLongReport) {
    const auto longReport = asLongReport(buildGetFeatureIndex(0x02, kFeatureUnifiedBattery));

    ASSERT_EQ(longReport.size(), kLongReportSize);
    EXPECT_EQ(longReport[0], kLongReportId);
    EXPECT_EQ(longReport[1], 0x02);
    EXPECT_EQ(longReport[4], 0x10);
    EXPECT_EQ(longReport[5], 0x04);
    EXPECT_EQ(longReport[19], 0x00);
}

TEST(HidppProtocolTest, ReadsDeviceIndexFromAnyFrame) {
    EXPECT_EQ(deviceIndexOf({kLongReportId, 0x02, 0x0D, 0x20, 0x00, 0x50, 0x01}), 0x02);
    EXPECT_EQ(deviceIndexOf({kShortReportId, 0xFF, 0x8F, 0x00, 0x0D, 0x01, 0x00}), 0xFF);
    EXPECT_EQ(deviceIndexOf({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}), -1); // not HID++
    EXPECT_EQ(deviceIndexOf({kShortReportId, 0x02}), -1);                      // too short
}

TEST(HidppProtocolTest, MatchesReplyToItsRequest) {
    const auto request = buildGetFeatureIndex(0x02, kFeatureUnifiedBattery);
    // Reply echoes feature index 0 and function byte. Long id is fine.
    EXPECT_TRUE(isReplyTo({kLongReportId, 0x02, 0x00, kSoftwareId, 0x09, 0x00, 0x05}, request));

    const auto call = buildFunctionCall(0x02, 0x09, 0x01);
    EXPECT_TRUE(isReplyTo({kLongReportId, 0x02, 0x09, 0x1D, 0x5F, 0x08, 0x00}, call));
}

TEST(HidppProtocolTest, RejectsNotificationsOtherDevicesAndOtherRequests) {
    const auto request = buildGetFeatureIndex(0x02, kFeatureUnifiedBattery);
    // Battery notification carries software id 0.
    EXPECT_FALSE(isReplyTo({kLongReportId, 0x02, 0x0D, 0x20, 0x00, 0x50, 0x01}, request));
    // Same shape but other device index.
    EXPECT_FALSE(isReplyTo({kLongReportId, 0x01, 0x00, kSoftwareId, 0x09, 0x00, 0x05}, request));
    // Reply to a different feature or function.
    EXPECT_FALSE(isReplyTo({kLongReportId, 0x02, 0x09, 0x1D, 0x5F, 0x08, 0x00}, request));
    EXPECT_FALSE(isReplyTo({kLongReportId, 0x02, 0x00, 0x1D, 0x09, 0x00, 0x05}, request));
}

TEST(HidppProtocolTest, MatchesErrorRepliesToTheirRequest) {
    const auto request = buildGetFeatureIndex(0xFF, kFeatureUnifiedBattery);
    // HID++ 1.0 receiver error: 0x8F, then sub id and address of the request.
    const std::vector<std::uint8_t> legacyError{kShortReportId, 0xFF, 0x8F, 0x00, kSoftwareId, 0x01, 0x00};
    EXPECT_TRUE(isReplyTo(legacyError, request));
    EXPECT_TRUE(isError(legacyError));

    // HID++ 2.0 error: 0xFF, then feature index and function of the request.
    const auto call = buildFunctionCall(0x02, 0x02, 0x02);
    const std::vector<std::uint8_t> error{kLongReportId, 0x02, 0xFF, 0x02, 0x2D, 0x02, 0x00};
    EXPECT_TRUE(isReplyTo(error, call));
    EXPECT_TRUE(isError(error));
    EXPECT_FALSE(isReplyTo(error, request));
}

TEST(HidppProtocolTest, DetectsErrorReports) {
    std::vector<std::uint8_t> error{kShortReportId, 0xFF, 0xFF, kSoftwareId, 0x01, 0x00, 0x00};
    EXPECT_TRUE(isError(error));
    EXPECT_EQ(featureIndexOf(error), 0);
}

TEST(HidppProtocolTest, ReadsTheFeatureIndex) {
    std::vector<std::uint8_t> reply{kShortReportId, 0xFF, 0x00, kSoftwareId, 0x06, 0x00, 0x00};
    EXPECT_EQ(featureIndexOf(reply), 0x06);

    // Index 0 means device lacks feature.
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

    std::vector<std::uint8_t> legacyError{kShortReportId, 0x01, 0x8F, 0x00, 0x0D, 0x09, 0x00};
    EXPECT_EQ(legacyBatteryPercent(legacyError), -1);
    EXPECT_EQ(featureIndexOf(legacyError), 0);
}

TEST(HidppProtocolTest, ReadsChargingState) {
    // Unified: byte 2 is 0 discharging, 1 charging, 2 slow, 3 complete.
    EXPECT_FALSE(unifiedBatteryCharging({kLongReportId, 0x02, 0x09, 0x1D, 95, 0x08, 0x00, 0x00}));
    EXPECT_TRUE(unifiedBatteryCharging({kLongReportId, 0x02, 0x09, 0x1D, 60, 0x04, 0x01, 0x01}));
    EXPECT_TRUE(unifiedBatteryCharging({kLongReportId, 0x02, 0x09, 0x1D, 60, 0x04, 0x02, 0x01}));
    EXPECT_FALSE(unifiedBatteryCharging({kLongReportId, 0x02, 0x09, 0x1D, 100, 0x08, 0x03, 0x01}));

    // Legacy: byte 2 is 0 discharging, 1 recharging, 3 complete, 4 slow.
    EXPECT_FALSE(legacyBatteryCharging({kShortReportId, 0xFF, 0x05, 0x0D, 40, 0x50, 0x00}));
    EXPECT_TRUE(legacyBatteryCharging({kShortReportId, 0xFF, 0x05, 0x0D, 40, 0x50, 0x01}));
    EXPECT_TRUE(legacyBatteryCharging({kShortReportId, 0xFF, 0x05, 0x0D, 40, 0x50, 0x04}));
    EXPECT_FALSE(legacyBatteryCharging({kShortReportId, 0xFF, 0x05, 0x0D, 100, 0x50, 0x03}));

    EXPECT_FALSE(unifiedBatteryCharging({kLongReportId, 0x02, 0xFF, 0x09, 0x1D, 0x01, 0x00}));
}

TEST(HidppProtocolTest, DecodesDeviceName) {
    EXPECT_EQ(deviceNameLength({kLongReportId, 0x02, 0x03, 0x0D, 24, 0x00, 0x00}), 24);
    EXPECT_EQ(deviceNameLength({kLongReportId, 0x02, 0xFF, 0x03, 0x0D, 0x02, 0x00}), 0);

    const std::vector<std::uint8_t> first{kLongReportId, 0x02, 0x03, 0x1D, 'M', 'X', ' ', 'M', 'a', 's',
                                          't', 'e', 'r', ' ', '4', ' ', 'f', 'o', 'r', ' '};
    EXPECT_EQ(deviceNameChunk(first), "MX Master 4 for ");

    // Short tail pads with NUL.
    const std::vector<std::uint8_t> second{kLongReportId, 0x02, 0x03, 0x1D, 'B', 'u', 's', 'i', 'n', 'e',
                                           's', 's', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(deviceNameChunk(second), "Business");

    EXPECT_EQ(deviceNameChunk({kLongReportId, 0x02, 0xFF, 0x03, 0x1D, 0x02, 0x00}), "");
}

TEST(HidppProtocolTest, MapsDeviceTypeToCategory) {
    auto typeReply = [](std::uint8_t type) {
        return std::vector<std::uint8_t>{kLongReportId, 0x02, 0x03, 0x2D, type, 0x00, 0x00};
    };
    EXPECT_EQ(deviceTypeCategory(typeReply(0x00)), "keyboard");
    EXPECT_EQ(deviceTypeCategory(typeReply(0x02)), "keyboard");
    EXPECT_EQ(deviceTypeCategory(typeReply(0x03)), "mouse");
    EXPECT_EQ(deviceTypeCategory(typeReply(0x04)), "mouse");
    EXPECT_EQ(deviceTypeCategory(typeReply(0x05)), "mouse");
    EXPECT_EQ(deviceTypeCategory(typeReply(0x08)), "headset");
    EXPECT_EQ(deviceTypeCategory(typeReply(0x0C)), "gamepad");
    EXPECT_EQ(deviceTypeCategory(typeReply(0x07)), ""); // receiver
    EXPECT_EQ(deviceTypeCategory({kLongReportId, 0x02, 0xFF, 0x03, 0x2D, 0x02, 0x00}), "");
}
