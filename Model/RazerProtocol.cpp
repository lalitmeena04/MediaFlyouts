#include "RazerProtocol.h"

namespace razer {

namespace {
// Offsets inside 90-byte report. Shifted for leading report-id byte.
constexpr std::size_t kStatus = 1;
constexpr std::size_t kTransactionId = 2;
constexpr std::size_t kDataSize = 6;
constexpr std::size_t kCommandClass = 7;
constexpr std::size_t kCommandId = 8;
constexpr std::size_t kArguments = 9;
constexpr std::size_t kChecksum = 89;
} // namespace

std::uint8_t checksum(const std::vector<std::uint8_t>& buffer) {
    if(buffer.size() < kBufferSize)
        return 0;
    std::uint8_t crc = 0;
    for(std::size_t i = 3; i < 89; ++i) // report bytes 2..87
        crc ^= buffer[i];
    return crc;
}

std::vector<std::uint8_t> buildRequest(std::uint8_t transactionId, std::uint8_t commandId) {
    std::vector<std::uint8_t> buffer(kBufferSize, 0);
    buffer[kStatus] = static_cast<std::uint8_t>(Status::NewCommand);
    buffer[kTransactionId] = transactionId;
    buffer[kDataSize] = 0x02;
    buffer[kCommandClass] = kCommandClassPower;
    buffer[kCommandId] = commandId;
    buffer[kChecksum] = checksum(buffer);
    return buffer;
}

Status statusOf(const std::vector<std::uint8_t>& buffer) {
    if(buffer.size() < kBufferSize)
        return Status::Failure;
    return static_cast<Status>(buffer[kStatus]);
}

bool isReplyTo(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId, std::uint8_t commandId) {
    if(buffer.size() < kBufferSize)
        return false;
    return buffer[kTransactionId] == transactionId && buffer[kCommandClass] == kCommandClassPower
           && buffer[kCommandId] == commandId;
}

int percentFromRaw(std::uint8_t raw) {
    return (static_cast<int>(raw) * 100 + 127) / 255;
}

int batteryPercent(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId) {
    if(!isReplyTo(buffer, transactionId, kCommandBatteryLevel) || statusOf(buffer) != Status::Successful)
        return -1;
    return percentFromRaw(buffer[kArguments + 1]);
}

bool chargingState(const std::vector<std::uint8_t>& buffer, std::uint8_t transactionId, bool& supported) {
    supported = false;
    if(!isReplyTo(buffer, transactionId, kCommandChargingStatus))
        return false;
    if(statusOf(buffer) != Status::Successful)
        return false;
    supported = true;
    return buffer[kArguments + 1] != 0;
}

} // namespace razer
