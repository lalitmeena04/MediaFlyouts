#pragma once

#include <cstdint>
#include <vector>

// Pure DualSense Bluetooth/USB HID protocol logic (CRC32 signing and report parsing),
// factored out of PlayStationControllerBatteryProvider so it's testable without a real
// controller or any Windows HID APIs.
namespace DualSenseProtocol {

// Standard CRC-32 (IEEE 802.3 / zlib polynomial), matching the one DualSense's
// Bluetooth protocol signs its output reports with.
uint32_t crc32(const std::vector<uint8_t>& data);

// Over Bluetooth, DualSense defaults to a "simple" input report (id 0x01) with no
// battery/touchpad data until the host sends this signed output report requesting the
// extended report (id 0x31) - same trick DS4Windows uses (SendInitialBTOutputReport).
// Returns the full 78-byte output report, CRC already computed and appended.
std::vector<uint8_t> buildEnableExtendedReportPacket();

// DualSense extended (0x31) input report layout (reverse-engineered by the
// DS4Windows/hid-playstation projects): byte 53 (USB) / 54 (Bluetooth, +1 offset)
// packs battery level in the low nibble (0-8, scaled to %), bit 0x20 = fully charged.
// Returns -1 if the report doesn't carry battery data (e.g. still on the BT "simple"
// report, or too short).
int parseBatteryPercent(const std::vector<uint8_t>& report, bool isBluetooth);

} // namespace DualSenseProtocol
