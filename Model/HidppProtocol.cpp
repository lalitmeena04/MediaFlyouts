#include "HidppProtocol.h"

namespace hidpp {

namespace {
constexpr std::size_t kReportId = 0;
constexpr std::size_t kDeviceIndex = 1;
constexpr std::size_t kFeatureIndex = 2;
constexpr std::size_t kFunction = 3;
constexpr std::size_t kParameters = 4;

constexpr std::uint8_t kRootFeatureIndex = 0x00;
constexpr std::uint8_t kErrorFeatureIndex = 0xFF;
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
                                            std::uint8_t function) {
    std::vector<std::uint8_t> report(kShortReportSize, 0);
    report[kReportId] = kShortReportId;
    report[kDeviceIndex] = deviceIndex;
    report[kFeatureIndex] = featureIndex;
    report[kFunction] = static_cast<std::uint8_t>((function << 4) | kSoftwareId);
    return report;
}

bool isReplyFor(const std::vector<std::uint8_t>& report, std::uint8_t deviceIndex) {
    if(report.size() < kShortReportSize)
        return false;
    if(report[kReportId] != kShortReportId && report[kReportId] != kLongReportId)
        return false;
    if(report[kDeviceIndex] != deviceIndex)
        return false;
    return (report[kFunction] & 0x0F) == kSoftwareId;
}

bool isError(const std::vector<std::uint8_t>& report) {
    return report.size() >= kShortReportSize && report[kFeatureIndex] == kErrorFeatureIndex;
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

int legacyBatteryPercent(const std::vector<std::uint8_t>& report) {
    if(report.size() < kShortReportSize || isError(report))
        return -1;
    const int percent = report[kParameters];
    return (percent >= 0 && percent <= 100) ? percent : -1;
}

} // namespace hidpp
