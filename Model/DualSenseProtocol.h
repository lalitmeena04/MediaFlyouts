#pragma once

#include <cstdint>
#include <vector>

namespace DualSenseProtocol {

uint32_t crc32(const std::vector<uint8_t>& data);

// Bluetooth DualSense starts with simple input report id 0x01.
// This packet enables extended report id 0x31.
std::vector<uint8_t> buildEnableExtendedReportPacket();

// DualSense extended report id 0x31 battery layout.
// Byte 53 for USB. Byte 54 for Bluetooth. Low nibble is level 0-8.
// Bit 0x20 means fully charged. Return -1 when battery data is absent.
int parseBatteryPercent(const std::vector<uint8_t>& report, bool isBluetooth);

} // namespace DualSenseProtocol
