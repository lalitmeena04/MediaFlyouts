#pragma once

#include <cstdint>
#include <vector>

// Parse Google Fast Pair message stream over RFCOMM.
// Keep free of Windows and Qt types for tests.
//
// Frame layout: [group][code][len_hi][len_lo][payload...]
namespace fastpair {

inline constexpr std::uint8_t kGroupDeviceInformation = 0x03;
inline constexpr std::uint8_t kCodeBatteryUpdated = 0x03;

// One battery component. Earbuds can report left, right, and case.
struct BatteryComponent {
        int percent = -1; // -1 means unknown level.
        bool charging = false;
};

struct Frame {
        std::uint8_t group = 0;
        std::uint8_t code = 0;
        std::vector<std::uint8_t> payload;
};

// Split byte stream into complete frames.
// Leave partial frame bytes in buffer.
std::vector<Frame> drainFrames(std::vector<std::uint8_t>& buffer);

bool isBatteryFrame(const Frame& frame);

// Decode battery frame payload. One byte per component.
// Bit 7 = charging. Low 7 bits = percent. 0x7F/0xFF = unknown.
std::vector<BatteryComponent> parseBattery(const Frame& frame);

// Surface lowest known component level. Return -1 when none is known.
int lowestPercent(const std::vector<BatteryComponent>& components);

bool anyCharging(const std::vector<BatteryComponent>& components);

} // namespace fastpair
