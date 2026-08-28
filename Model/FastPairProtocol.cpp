#include "FastPairProtocol.h"

namespace fastpair {

std::vector<Frame> drainFrames(std::vector<std::uint8_t>& buffer) {
    std::vector<Frame> frames;
    std::size_t offset = 0;
    while(buffer.size() - offset >= 4) {
        const std::size_t length = (static_cast<std::size_t>(buffer[offset + 2]) << 8) | buffer[offset + 3];
        if(buffer.size() - offset - 4 < length)
            break; // rest of this frame hasn't arrived yet

        Frame frame;
        frame.group = buffer[offset];
        frame.code = buffer[offset + 1];
        frame.payload.assign(buffer.begin() + static_cast<std::ptrdiff_t>(offset + 4),
                             buffer.begin() + static_cast<std::ptrdiff_t>(offset + 4 + length));
        frames.push_back(std::move(frame));
        offset += 4 + length;
    }
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(offset));
    return frames;
}

bool isBatteryFrame(const Frame& frame) {
    return frame.group == kGroupDeviceInformation && frame.code == kCodeBatteryUpdated && !frame.payload.empty();
}

std::vector<BatteryComponent> parseBattery(const Frame& frame) {
    std::vector<BatteryComponent> components;
    if(!isBatteryFrame(frame))
        return components;

    for(std::uint8_t raw : frame.payload) {
        BatteryComponent component;
        component.charging = (raw & 0x80) != 0;
        const int level = raw & 0x7F;
        component.percent = (level > 100) ? -1 : level;
        components.push_back(component);
    }
    return components;
}

int lowestPercent(const std::vector<BatteryComponent>& components) {
    int lowest = -1;
    for(const BatteryComponent& component : components) {
        if(component.percent < 0)
            continue;
        if(lowest < 0 || component.percent < lowest)
            lowest = component.percent;
    }
    return lowest;
}

bool anyCharging(const std::vector<BatteryComponent>& components) {
    for(const BatteryComponent& component : components)
        if(component.charging)
            return true;
    return false;
}

} // namespace fastpair
