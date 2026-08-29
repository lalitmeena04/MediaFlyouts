#pragma once

#include <cstdint>
#include <vector>

// Logitech HID++ 2.0, the protocol used by Unifying / Lightspeed / Bolt receivers and
// their devices. Kept free of Windows/Qt types so it can be unit tested.
namespace hidpp {

inline constexpr std::uint8_t kShortReportId = 0x10;
inline constexpr std::uint8_t kLongReportId = 0x11;
inline constexpr std::size_t kShortReportSize = 7;
inline constexpr std::size_t kLongReportSize = 20;

// device index 0xFF addresses a directly-attached device; 0x01..0x06 are receiver slots
inline constexpr std::uint8_t kDirectDeviceIndex = 0xFF;
inline constexpr std::uint8_t kMaxReceiverSlot = 0x06;

// arbitrary 4-bit software id, echoed back so we can tell our replies from notifications
inline constexpr std::uint8_t kSoftwareId = 0x0D;

inline constexpr std::uint16_t kFeatureBatteryStatus = 0x1000;  // older devices
inline constexpr std::uint16_t kFeatureUnifiedBattery = 0x1004; // newer devices

// Root feature, function 0: resolves a feature id to the index this device uses for it.
std::vector<std::uint8_t> buildGetFeatureIndex(std::uint8_t deviceIndex, std::uint16_t featureId);

std::vector<std::uint8_t> buildFunctionCall(std::uint8_t deviceIndex, std::uint8_t featureIndex,
                                            std::uint8_t function);

// True when the report is a HID++ reply to us rather than an unrelated notification.
bool isReplyFor(const std::vector<std::uint8_t>& report, std::uint8_t deviceIndex);

// Devices answer an unsupported feature with an error report instead of a result.
bool isError(const std::vector<std::uint8_t>& report);

// Feature index from a getFeatureIndex reply; 0 means the device doesn't support it.
std::uint8_t featureIndexOf(const std::vector<std::uint8_t>& report);

// State of charge from a 0x1004 UnifiedBattery reply, or -1 when unavailable.
int unifiedBatteryPercent(const std::vector<std::uint8_t>& report);

// Battery percentage from a legacy 0x1000 BatteryStatus reply, or -1 when unavailable.
int legacyBatteryPercent(const std::vector<std::uint8_t>& report);

} // namespace hidpp
