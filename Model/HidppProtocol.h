#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hidpp {

inline constexpr std::uint8_t kShortReportId = 0x10;
inline constexpr std::uint8_t kLongReportId = 0x11;
inline constexpr std::size_t kShortReportSize = 7;
inline constexpr std::size_t kLongReportSize = 20;

// Device index 0xFF is directly attached. 0x01..0x06 are receiver slots.
inline constexpr std::uint8_t kDirectDeviceIndex = 0xFF;
inline constexpr std::uint8_t kMaxReceiverSlot = 0x06;

// Arbitrary 4-bit software id. Replies echo it back.
inline constexpr std::uint8_t kSoftwareId = 0x0D;

inline constexpr std::uint16_t kFeatureDeviceName = 0x0005;     // name and type
inline constexpr std::uint16_t kFeatureBatteryStatus = 0x1000;  // older devices
inline constexpr std::uint16_t kFeatureUnifiedBattery = 0x1004; // newer devices

// Root feature, function 0: resolves a feature id to the index this device uses for it.
std::vector<std::uint8_t> buildGetFeatureIndex(std::uint8_t deviceIndex, std::uint16_t featureId);

std::vector<std::uint8_t> buildFunctionCall(std::uint8_t deviceIndex, std::uint8_t featureIndex,
                                            std::uint8_t function,
                                            const std::vector<std::uint8_t>& params = {});

// Rewrite a request as a long report. Many devices answer only long reports.
std::vector<std::uint8_t> asLongReport(std::vector<std::uint8_t> report);

int deviceIndexOf(const std::vector<std::uint8_t>& report);

bool isReplyTo(const std::vector<std::uint8_t>& report, const std::vector<std::uint8_t>& request);

// Unsupported feature returns error report. Covers HID++ 1.0 and 2.0 error frames.
bool isError(const std::vector<std::uint8_t>& report);

// Feature index from getFeatureIndex reply. 0 means unsupported.
std::uint8_t featureIndexOf(const std::vector<std::uint8_t>& report);

int unifiedBatteryPercent(const std::vector<std::uint8_t>& report);

bool unifiedBatteryCharging(const std::vector<std::uint8_t>& report);

int legacyBatteryPercent(const std::vector<std::uint8_t>& report);

bool legacyBatteryCharging(const std::vector<std::uint8_t>& report);

int deviceNameLength(const std::vector<std::uint8_t>& report);

std::string deviceNameChunk(const std::vector<std::uint8_t>& report);

std::string deviceTypeCategory(const std::vector<std::uint8_t>& report);

} // namespace hidpp
