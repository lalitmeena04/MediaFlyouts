#pragma once

#include <cstdint>
#include <vector>

// Razer's 90-byte HID feature-report protocol (as documented by the OpenRazer project).
// Kept free of Windows/Qt types so it can be unit tested.
namespace razer {

inline constexpr std::size_t kReportSize = 90;
// the Win32 HID API expects a leading report-id byte in front of the report itself
inline constexpr std::size_t kBufferSize = kReportSize + 1;

inline constexpr std::uint8_t kCommandClassPower = 0x07;
inline constexpr std::uint8_t kCommandBatteryLevel = 0x80;
inline constexpr std::uint8_t kCommandChargingStatus = 0x84;

enum class Status : std::uint8_t {
    NewCommand = 0x00,
    Busy = 0x01,
    Successful = 0x02,
    Failure = 0x03,
    Timeout = 0x04,
    NotSupported = 0x05,
};

// XOR of report bytes 2..87.
std::uint8_t checksum(const std::vector<std::uint8_t>& buffer);

// Builds a request buffer (report id byte + 90-byte report) for a power-class command.
std::vector<std::uint8_t> buildRequest(std::uint8_t transactionId, std::uint8_t commandId);

Status statusOf(const std::vector<std::uint8_t>& buffer);

// True when the buffer is the device's answer to the given request rather than one of the
// unrelated reports it also emits.
bool isReplyTo(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId, std::uint8_t commandId);

// Battery level is reported on a 0-255 scale, not 0-100.
int percentFromRaw(std::uint8_t raw);

// Extracts the battery percentage from a successful reply, or -1 if it isn't one.
int batteryPercent(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId);

// Extracts charging state from a successful reply; 'supported' is false when the device
// answers NotSupported, which several wireless models do.
bool chargingState(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId, bool& supported);

} // namespace razer
