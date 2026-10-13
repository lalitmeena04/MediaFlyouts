#pragma once

#include <cstdint>
#include <vector>

// Frame layout: [group][code][len_hi][len_lo][payload...]
namespace fastpair {

inline constexpr std::uint8_t kGroupDeviceInformation = 0x03;
inline constexpr std::uint8_t kCodeBatteryUpdated = 0x03;

struct BatteryComponent {
        int percent = -1;
        bool charging = false;
};

struct Frame {
        std::uint8_t group = 0;
        std::uint8_t code = 0;
        std::vector<std::uint8_t> payload;
};

std::vector<Frame> drainFrames(std::vector<std::uint8_t>& buffer);

bool isBatteryFrame(const Frame& frame);

// Bit 7 = charging. Low 7 bits = percent. 0x7F/0xFF = unknown.
std::vector<BatteryComponent> parseBattery(const Frame& frame);

int lowestPercent(const std::vector<BatteryComponent>& components);

bool anyCharging(const std::vector<BatteryComponent>& components);

} // namespace fastpair
