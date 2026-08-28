#include "DualSenseProtocol.h"

#include <algorithm>

namespace DualSenseProtocol {

uint32_t crc32(const std::vector<uint8_t>& data) {
    static uint32_t table[256];
    static bool initialized = false;
    if(!initialized) {
        for(uint32_t i = 0; i < 256; ++i) {
            uint32_t c = i;
            for(int k = 0; k < 8; ++k)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        initialized = true;
    }
    uint32_t crc = 0xFFFFFFFFu;
    for(uint8_t b : data)
        crc = table[(crc ^ b) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

std::vector<uint8_t> buildEnableExtendedReportPacket() {
    std::vector<uint8_t> report(78, 0);
    report[0] = 0x31; // BT output report id
    report[1] = 0x02; // "data" sub-id
    report[2] = 0x15; // toggle all LED lights, just to force a report state change

    std::vector<uint8_t> crcInput;
    crcInput.reserve(1 + 74);
    crcInput.push_back(0xA2); // Bluetooth output-report CRC seed byte
    crcInput.insert(crcInput.end(), report.begin(), report.begin() + 74);
    const uint32_t crc = crc32(crcInput);
    report[74] = static_cast<uint8_t>(crc);
    report[75] = static_cast<uint8_t>(crc >> 8);
    report[76] = static_cast<uint8_t>(crc >> 16);
    report[77] = static_cast<uint8_t>(crc >> 24);

    return report;
}

int parseBatteryPercent(const std::vector<uint8_t>& report, bool isBluetooth) {
    if(isBluetooth && (report.empty() || report[0] != 0x31))
        return -1; // still on the basic report; caller should retry after enabling extended mode

    const size_t idx = 53 + (isBluetooth ? 1 : 0);
    if(report.size() <= idx)
        return -1;

    const uint8_t b = report[idx];
    if(b & 0x20)
        return 100;
    return std::min((b & 0x0F) * 100 / 8, 100);
}

} // namespace DualSenseProtocol
