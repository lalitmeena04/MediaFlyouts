#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Jabra GN Protocol (GNP) over HID. Same frames for a Link dongle and a headset on USB.
// Frame: 05 <dst> <src> <seq> <type<<6 | len> <cmd> <sub> <payload...>, zero padded to 64 bytes.
namespace gnp {

inline constexpr std::uint16_t kVendorId = 0x0B0E;
inline constexpr std::uint16_t kVendorUsagePage = 0xFF00;
// The vendor collection carrying report 5 differs: 0x05 on a Link dongle, 0x01 on a headset cable.
inline constexpr std::uint16_t kDongleUsage = 0x05;
inline constexpr std::uint16_t kHeadsetUsage = 0x01;

inline constexpr std::uint8_t kReportId = 0x05;
inline constexpr std::size_t kFrameSize = 64;
inline constexpr std::size_t kHeaderSize = 7;

inline constexpr std::uint8_t kAddressHost = 0x00;
inline constexpr std::uint8_t kAddressDongle = 0x01;
inline constexpr std::uint8_t kAddressHeadsetViaDongle = 0x04;
inline constexpr std::uint8_t kAddressHeadsetOnUsb = 0x08;

inline constexpr std::uint8_t kCommandIdentity = 0x02;
inline constexpr std::uint8_t kIdentityName = 0x00;
inline constexpr std::uint8_t kCommandStatus = 0x12;
inline constexpr std::uint8_t kStatusHeadsetBattery = 0x02;

// Headset address for a vendor collection usage. Zero means not a GNP collection.
std::uint8_t headsetAddressForUsage(std::uint16_t usagePage, std::uint16_t usage);

std::vector<std::uint8_t> buildGet(std::uint8_t address, std::uint8_t sequence, std::uint8_t command,
                                   std::uint8_t subCommand);

// Response frame with the sequence of the request. Unsolicited events never match.
bool isReplyTo(const std::vector<std::uint8_t>& frame, const std::vector<std::uint8_t>& request);

bool isNack(const std::vector<std::uint8_t>& frame);

// NACK reason says the headset is off or out of range, not that the command is unknown.
bool nackMeansOffline(const std::vector<std::uint8_t>& frame);

std::vector<std::uint8_t> payloadOf(const std::vector<std::uint8_t>& frame);

// Battery reply: [flags][percent][adc hi][adc lo]. Returns -1 when the reply is unusable.
int batteryPercent(const std::vector<std::uint8_t>& frame);

bool batteryCharging(const std::vector<std::uint8_t>& frame);

// Identity reply: [len][chars], length prefix optional. Empty when the reply is unusable.
std::string identityString(const std::vector<std::uint8_t>& frame);

} // namespace gnp
