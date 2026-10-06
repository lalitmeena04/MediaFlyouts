#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Logitech HID++ 2.0 protocol for Unifying, Lightspeed, and Bolt.
// Kept free of Windows and Qt types for tests.
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

// Device index carried by any HID++ frame. -1 for non HID++ frames.
int deviceIndexOf(const std::vector<std::uint8_t>& report);

// True when report answers this request. Error replies count too.
// Notifications and replies to other requests do not match.
bool isReplyTo(const std::vector<std::uint8_t>& report, const std::vector<std::uint8_t>& request);

// Unsupported feature returns error report. Covers HID++ 1.0 and 2.0 error frames.
bool isError(const std::vector<std::uint8_t>& report);

// Feature index from getFeatureIndex reply. 0 means unsupported.
std::uint8_t featureIndexOf(const std::vector<std::uint8_t>& report);

// State of charge from a 0x1004 UnifiedBattery reply, or -1 when unavailable.
int unifiedBatteryPercent(const std::vector<std::uint8_t>& report);

// True when a 0x1004 getStatus reply says the battery charges now.
bool unifiedBatteryCharging(const std::vector<std::uint8_t>& report);

// Battery percentage from a legacy 0x1000 BatteryStatus reply, or -1 when unavailable.
int legacyBatteryPercent(const std::vector<std::uint8_t>& report);

// True when a 0x1000 getStatus reply says the battery charges now.
bool legacyBatteryCharging(const std::vector<std::uint8_t>& report);

// Name length from a 0x0005 function 0 reply. 0 when unavailable.
int deviceNameLength(const std::vector<std::uint8_t>& report);

// Name characters from a 0x0005 function 1 reply. Stops at first NUL.
std::string deviceNameChunk(const std::vector<std::uint8_t>& report);

// Icon category from a 0x0005 function 2 reply. Empty when unknown.
std::string deviceTypeCategory(const std::vector<std::uint8_t>& report);

} // namespace hidpp
