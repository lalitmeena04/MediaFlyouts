#include "HidppProtocol.h"

#include <algorithm>
#include <initializer_list>

namespace hidpp {

namespace {
constexpr std::size_t kReportId = 0;
constexpr std::size_t kDeviceIndex = 1;
constexpr std::size_t kFeatureIndex = 2;
constexpr std::size_t kFunction = 3;
constexpr std::size_t kParameters = 4;

constexpr std::uint8_t kRootFeatureIndex = 0x00;
constexpr std::uint8_t kErrorFeatureIndex = 0xFF; // HID++ 2.0 error frame
constexpr std::uint8_t kLegacyErrorSubId = 0x8F;  // HID++ 1.0 error frame

bool isHidppFrame(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize)
        return false;
    return report[kReportId] == kShortReportId || report[kReportId] == kLongReportId;
}

bool chargingFromStatus(const std::vector<std::uint8_t>& report, std::size_t offset,
                        std::initializer_list<std::uint8_t> chargingStates) {
    if(report.size() <= kParameters + offset || isError(report))
        return false;
    const std::uint8_t status = report[kParameters + offset];
    return std::find(chargingStates.begin(), chargingStates.end(), status) != chargingStates.end();
}
} // namespace

std::vector<std::uint8_t> buildGetFeatureIndex(std::uint8_t deviceIndex, std::uint16_t featureId) {
    std::vector<std::uint8_t> report(kShortReportSize, 0);
    report[kReportId] = kShortReportId;
    report[kDeviceIndex] = deviceIndex;
    report[kFeatureIndex] = kRootFeatureIndex;
    report[kFunction] = kSoftwareId; // function 0, low nibble carries the software id
    report[kParameters] = static_cast<std::uint8_t>(featureId >> 8);
    report[kParameters + 1] = static_cast<std::uint8_t>(featureId & 0xFF);
    return report;
}

std::vector<std::uint8_t> buildFunctionCall(std::uint8_t deviceIndex, std::uint8_t featureIndex,
                                            std::uint8_t function, const std::vector<std::uint8_t>& params) {
    std::vector<std::uint8_t> report(std::max(kShortReportSize, kParameters + params.size()), 0);
    report[kReportId] = report.size() > kShortReportSize ? kLongReportId : kShortReportId;
    report[kDeviceIndex] = deviceIndex;
    report[kFeatureIndex] = featureIndex;
    report[kFunction] = static_cast<std::uint8_t>((function << 4) | kSoftwareId);
    std::copy(params.begin(), params.end(), report.begin() + kParameters);
    return report;
}

std::vector<std::uint8_t> asLongReport(std::vector<std::uint8_t> report) {
    if(report.empty())
        return report;
    report[kReportId] = kLongReportId;
    report.resize(kLongReportSize, 0);
    return report;
}

int deviceIndexOf(const std::vector<std::uint8_t>& report) {
    return isHidppFrame(report) ? report[kDeviceIndex] : -1;
}

bool isReplyTo(const std::vector<std::uint8_t>& report, const std::vector<std::uint8_t>& request) {
    if(!isHidppFrame(report) || request.size() < kParameters)
        return false;
    if(report[kDeviceIndex] != request[kDeviceIndex])
        return false;
    // Error frames echo feature index and function one byte later.
    if(isError(report))
        return report[kFunction] == request[kFeatureIndex] && report[kParameters] == request[kFunction];
    return report[kFeatureIndex] == request[kFeatureIndex] && report[kFunction] == request[kFunction];
}

bool isError(const std::vector<std::uint8_t>& report) {
    return report.size() >= kShortReportSize
           && (report[kFeatureIndex] == kErrorFeatureIndex || report[kFeatureIndex] == kLegacyErrorSubId);
}

std::uint8_t featureIndexOf(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize || isError(report))
        return 0;
    return report[kParameters];
}

int unifiedBatteryPercent(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize || isError(report))
        return -1;
    const int percent = report[kParameters];
    return (percent >= 0 && percent <= 100) ? percent : -1;
}

bool unifiedBatteryCharging(const std::vector<std::uint8_t>& report) {
    // Byte 2: 1 charging, 2 slow charging. 3 means complete.
    return chargingFromStatus(report, 2, {0x01, 0x02});
}

int legacyBatteryPercent(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize || isError(report))
        return -1;
    const int percent = report[kParameters];
    return (percent >= 0 && percent <= 100) ? percent : -1;
}

bool legacyBatteryCharging(const std::vector<std::uint8_t>& report) {
    // Byte 2: 1 recharging, 2 almost complete, 4 slow recharging.
    return chargingFromStatus(report, 2, {0x01, 0x02, 0x04});
}

int deviceNameLength(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize || isError(report))
        return 0;
    return report[kParameters];
}

std::string deviceNameChunk(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize || isError(report))
        return {};
    std::string chunk;
    for(std::size_t i = kParameters; i < report.size() && report[i] != 0; ++i)
        chunk.push_back(static_cast<char>(report[i]));
    return chunk;
}

std::string deviceTypeCategory(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize || isError(report))
        return {};
    switch(report[kParameters]) {
    case 0x00: // keyboard
    case 0x02: return "keyboard"; // numpad
    case 0x03: // mouse
    case 0x04: // trackpad
    case 0x05: return "mouse"; // trackball
    case 0x08: return "headset";
    case 0x0B: // joystick
    case 0x0C: return "gamepad";
    default: return {};
    }
}

} // namespace hidpp
