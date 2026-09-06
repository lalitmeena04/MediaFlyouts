#pragma once

#include <cstdint>
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

inline constexpr std::uint16_t kFeatureBatteryStatus = 0x1000;  // older devices
inline constexpr std::uint16_t kFeatureUnifiedBattery = 0x1004; // newer devices

// Root feature, function 0: resolves a feature id to the index this device uses for it.
std::vector<std::uint8_t> buildGetFeatureIndex(std::uint8_t deviceIndex, std::uint16_t featureId);

std::vector<std::uint8_t> buildFunctionCall(std::uint8_t deviceIndex, std::uint8_t featureIndex,
                                            std::uint8_t function);

// True when report is HID++ reply for this device.
bool isReplyFor(const std::vector<std::uint8_t>& report, std::uint8_t deviceIndex);

// Unsupported feature returns error report.
bool isError(const std::vector<std::uint8_t>& report);

// Feature index from getFeatureIndex reply. 0 means unsupported.
std::uint8_t featureIndexOf(const std::vector<std::uint8_t>& report);

// State of charge from a 0x1004 UnifiedBattery reply, or -1 when unavailable.
int unifiedBatteryPercent(const std::vector<std::uint8_t>& report);

// Battery percentage from a legacy 0x1000 BatteryStatus reply, or -1 when unavailable.
int legacyBatteryPercent(const std::vector<std::uint8_t>& report);

} // namespace hidpp
