#pragma once

#include <cstdint>
#include <vector>

namespace razer {

inline constexpr std::size_t kReportSize = 90;
// Win32 HID API expects leading report-id byte.
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

std::vector<std::uint8_t> buildRequest(std::uint8_t transactionId, std::uint8_t commandId);

Status statusOf(const std::vector<std::uint8_t>& buffer);

bool isReplyTo(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId, std::uint8_t commandId);

// Battery level is reported on a 0-255 scale, not 0-100.
int percentFromRaw(std::uint8_t raw);

int batteryPercent(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId);

bool chargingState(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId, bool& supported);

} // namespace razer
