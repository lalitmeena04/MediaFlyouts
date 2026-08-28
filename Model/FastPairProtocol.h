#pragma once

#include <cstdint>
#include <vector>

// Parsing for the Google Fast Pair "message stream" carried over RFCOMM. Kept free of
// Windows/Qt types so it can be unit tested.
//
// Frame layout: [group][code][len_hi][len_lo][payload...]
namespace fastpair {

inline constexpr std::uint8_t kGroupDeviceInformation = 0x03;
inline constexpr std::uint8_t kCodeBatteryUpdated = 0x03;

// One battery component. A true wireless set reports left/right/case; a single-body
// device reports just one.
struct BatteryComponent {
        int percent = -1; // -1 when the device reports the level as unknown (0xFF)
        bool charging = false;
};

struct Frame {
        std::uint8_t group = 0;
        std::uint8_t code = 0;
        std::vector<std::uint8_t> payload;
};

// Splits a byte stream into complete frames. Trailing bytes of a partial frame are left
// in 'buffer' so the caller can append more data and parse again.
std::vector<Frame> drainFrames(std::vector<std::uint8_t>& buffer);

bool isBatteryFrame(const Frame& frame);

// Decodes a battery frame payload: one byte per component, bit 7 = charging,
// low 7 bits = percent (0x7F/0xFF meaning "unknown").
std::vector<BatteryComponent> parseBattery(const Frame& frame);

// The level we surface for a multi-component device: the lowest known component, since
// that is the one that will die first. Returns -1 when nothing is known.
int lowestPercent(const std::vector<BatteryComponent>& components);

bool anyCharging(const std::vector<BatteryComponent>& components);

} // namespace fastpair
