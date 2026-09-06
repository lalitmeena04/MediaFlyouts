#pragma once

#include <cstdint>
#include <vector>

// Pure DualSense Bluetooth and USB HID protocol logic.
// Tests do not need real controller or Windows HID APIs.
namespace DualSenseProtocol {

// Standard CRC-32. Matches DualSense Bluetooth output report signing.
uint32_t crc32(const std::vector<uint8_t>& data);

// Bluetooth DualSense starts with simple input report id 0x01.
// This packet enables extended report id 0x31.
// Return full 78-byte output report with CRC appended.
std::vector<uint8_t> buildEnableExtendedReportPacket();

// DualSense extended report id 0x31 battery layout.
// Byte 53 for USB. Byte 54 for Bluetooth. Low nibble is level 0-8.
// Bit 0x20 means fully charged. Return -1 when battery data is absent.
int parseBatteryPercent(const std::vector<uint8_t>& report, bool isBluetooth);

} // namespace DualSenseProtocol
