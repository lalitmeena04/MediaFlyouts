#include "JabraGnpProtocol.h"

#include <algorithm>

namespace gnp {

namespace {
constexpr std::size_t kReportIdIndex = 0;
constexpr std::size_t kDestinationIndex = 1;
constexpr std::size_t kSourceIndex = 2;
constexpr std::size_t kSequenceIndex = 3;
constexpr std::size_t kTypeLengthIndex = 4;
constexpr std::size_t kCommandIndex = 5;
constexpr std::size_t kSubCommandIndex = 6;

constexpr std::uint8_t kTypeGet = 0x01;
constexpr std::uint8_t kTypeResponse = 0x03;
// Length counts bytes from dst to the end of the payload. Six means no payload.
constexpr std::uint8_t kLengthNoPayload = 0x06;
constexpr std::uint8_t kLengthMask = 0x3F;

constexpr std::uint8_t kCommandNack = 0xFE;

constexpr std::uint8_t kNackPeerOffline = 0xF3;
constexpr std::uint8_t kNackNoChild = 0xF4;
constexpr std::uint8_t kNackNoDevice = 0xF5;

constexpr std::uint8_t kBatteryFlagCharging = 0x01;

bool isFrame(const std::vector<std::uint8_t>& frame) {
    return frame.size() >= kHeaderSize && frame[kReportIdIndex] == kReportId;
}

bool isResponse(const std::vector<std::uint8_t>& frame) {
    return isFrame(frame) && (frame[kTypeLengthIndex] >> 6) == kTypeResponse;
}
} // namespace

std::uint8_t headsetAddressForUsage(std::uint16_t usagePage, std::uint16_t usage) {
    if(usagePage != kVendorUsagePage)
        return 0;
    if(usage == kDongleUsage)
        return kAddressHeadsetViaDongle;
    if(usage == kHeadsetUsage)
        return kAddressHeadsetOnUsb;
    return 0;
}

std::vector<std::uint8_t> buildGet(std::uint8_t address, std::uint8_t sequence, std::uint8_t command,
                                   std::uint8_t subCommand) {
    std::vector<std::uint8_t> frame(kFrameSize, 0);
    frame[kReportIdIndex] = kReportId;
    frame[kDestinationIndex] = address;
    frame[kSourceIndex] = kAddressHost;
    frame[kSequenceIndex] = sequence;
    frame[kTypeLengthIndex] = static_cast<std::uint8_t>((kTypeGet << 6) | kLengthNoPayload);
    frame[kCommandIndex] = command;
    frame[kSubCommandIndex] = subCommand;
    return frame;
}

bool isReplyTo(const std::vector<std::uint8_t>& frame, const std::vector<std::uint8_t>& request) {
    if(!isResponse(frame) || request.size() < kHeaderSize)
        return false;
    return frame[kSequenceIndex] == request[kSequenceIndex];
}

bool isNack(const std::vector<std::uint8_t>& frame) {
    return isResponse(frame) && frame[kCommandIndex] == kCommandNack;
}

bool nackMeansOffline(const std::vector<std::uint8_t>& frame) {
    if(!isNack(frame))
        return false;
    const std::uint8_t reason = frame[kSubCommandIndex];
    return reason == kNackPeerOffline || reason == kNackNoChild || reason == kNackNoDevice;
}

std::vector<std::uint8_t> payloadOf(const std::vector<std::uint8_t>& frame) {
    if(!isResponse(frame) || isNack(frame))
        return {};
    const std::size_t length = frame[kTypeLengthIndex] & kLengthMask;
    if(length <= kLengthNoPayload)
        return {};
    const std::size_t payloadLength = std::min(length - kLengthNoPayload, frame.size() - kHeaderSize);
    return {frame.begin() + kHeaderSize, frame.begin() + kHeaderSize + payloadLength};
}

int batteryPercent(const std::vector<std::uint8_t>& frame) {
    const std::vector<std::uint8_t> payload = payloadOf(frame);
    if(payload.size() < 2)
        return -1;
    return std::min<int>(100, payload[1]);
}

bool batteryCharging(const std::vector<std::uint8_t>& frame) {
    const std::vector<std::uint8_t> payload = payloadOf(frame);
    return payload.size() >= 2 && (payload[0] & kBatteryFlagCharging) != 0;
}

std::string identityString(const std::vector<std::uint8_t>& frame) {
    const std::vector<std::uint8_t> payload = payloadOf(frame);
    if(payload.empty())
        return {};
    std::size_t start = 0;
    if(payload[0] == payload.size() - 1)
        start = 1;
    std::string text;
    for(std::size_t i = start; i < payload.size() && payload[i] != 0; ++i)
        text.push_back(static_cast<char>(payload[i]));
    return text;
}

} // namespace gnp
