#include "JabraBatteryProvider.h"

#include "HidCollections.h"
#include "JabraGnpProtocol.h"
#include "Logging.h"

#include <QSet>

#include <vector>

namespace {

constexpr DWORD kWriteTimeoutMs = 300;
constexpr DWORD kExchangeTimeoutMs = 1000;
// Offline headset is retried after this many polls. Dongle answers NACK fast, so the cost is small.
constexpr int kOfflineSkipPolls = 2;

bool writeReport(HANDLE device, const std::vector<std::uint8_t>& frame) {
    OVERLAPPED ov = {};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    DWORD written = 0;
    BOOL ok = WriteFile(device, frame.data(), static_cast<DWORD>(frame.size()), &written, &ov);
    if(!ok && GetLastError() == ERROR_IO_PENDING) {
        if(WaitForSingleObject(ov.hEvent, kWriteTimeoutMs) == WAIT_OBJECT_0)
            ok = GetOverlappedResult(device, &ov, &written, FALSE);
        else
            CancelIo(device);
    }
    CloseHandle(ov.hEvent);
    return ok && written == frame.size();
}

bool readReport(HANDLE device, USHORT length, DWORD timeoutMs, std::vector<std::uint8_t>& frame) {
    frame.assign(length, 0);
    OVERLAPPED ov = {};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    DWORD read = 0;
    BOOL ok = ReadFile(device, frame.data(), length, &read, &ov);
    if(!ok && GetLastError() == ERROR_IO_PENDING) {
        if(WaitForSingleObject(ov.hEvent, timeoutMs) == WAIT_OBJECT_0)
            ok = GetOverlappedResult(device, &ov, &read, FALSE);
        else
            CancelIo(device);
    }
    CloseHandle(ov.hEvent);
    if(!ok || read == 0)
        return false;
    frame.resize(read);
    return true;
}

enum class ExchangeResult { Reply, Offline, NoReply };

// Windows requires the exact output report length. Events between request and reply are skipped.
ExchangeResult exchange(HANDLE device, const HidCollection& collection, std::vector<std::uint8_t> request,
                        std::vector<std::uint8_t>& reply) {
    request.resize(collection.outputLength, 0);
    if(!writeReport(device, request))
        return ExchangeResult::NoReply;
    const ULONGLONG deadline = GetTickCount64() + kExchangeTimeoutMs;
    for(ULONGLONG now = GetTickCount64(); now < deadline; now = GetTickCount64()) {
        if(!readReport(device, collection.inputLength, static_cast<DWORD>(deadline - now), reply))
            return ExchangeResult::NoReply;
        if(!gnp::isReplyTo(reply, request))
            continue;
        if(gnp::isNack(reply))
            return gnp::nackMeansOffline(reply) ? ExchangeResult::Offline : ExchangeResult::NoReply;
        return ExchangeResult::Reply;
    }
    return ExchangeResult::NoReply;
}

} // namespace

bool jabraGnpProductPresent(quint16 productId) {
    for(const HidCollection& collection : enumerateHidCollections(gnp::kVendorId)) {
        if(collection.productId == productId
           && gnp::headsetAddressForUsage(collection.usagePage, collection.usage) != 0)
            return true;
    }
    return false;
}

QVector<BatteryDeviceInfo> JabraBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;
    QSet<QString> seenNames;

    for(const HidCollection& collection : enumerateHidCollections(gnp::kVendorId)) {
        const std::uint8_t address = gnp::headsetAddressForUsage(collection.usagePage, collection.usage);
        if(address == 0 || collection.outputLength == 0 || collection.inputLength == 0)
            continue;

        if(const auto skip = m_skipPolls.find(collection.path); skip != m_skipPolls.end()) {
            if(skip.value() > 0) {
                --skip.value();
                continue;
            }
            m_skipPolls.erase(skip);
        }

        HANDLE device = CreateFileW(reinterpret_cast<const wchar_t*>(collection.path.utf16()),
                                    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                    OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        if(device == INVALID_HANDLE_VALUE) {
            TRC_DEBUG("jabra: cannot open pid {:04X} usage {:04X}/{:04X}, error {}", collection.productId,
                      collection.usagePage, collection.usage, GetLastError());
            continue;
        }

        std::vector<std::uint8_t> reply;
        const ExchangeResult battery =
            exchange(device, collection,
                     gnp::buildGet(address, m_sequence++, gnp::kCommandStatus, gnp::kStatusHeadsetBattery), reply);
        if(battery != ExchangeResult::Reply) {
            TRC_DEBUG("jabra: pid {:04X} address {:02X} {}, skip {} polls", collection.productId, address,
                      battery == ExchangeResult::Offline ? "headset offline" : "no battery reply",
                      kOfflineSkipPolls);
            m_skipPolls.insert(collection.path, kOfflineSkipPolls);
            CloseHandle(device);
            continue;
        }

        BatteryDeviceInfo info;
        info.percent = gnp::batteryPercent(reply);
        info.charging = gnp::batteryCharging(reply);
        info.category = QStringLiteral("headset");
        // The dongle is a Bluetooth radio Windows cannot see. The cable is plain USB.
        info.connectionKind = address == gnp::kAddressHeadsetViaDongle ? QStringLiteral("bluetooth")
                                                                        : QStringLiteral("usb");
        info.name = collection.product.isEmpty() ? QStringLiteral("Jabra headset") : collection.product;

        // Product string names the dongle. The headset behind it knows its own name.
        if(exchange(device, collection,
                    gnp::buildGet(address, m_sequence++, gnp::kCommandIdentity, gnp::kIdentityName), reply)
           == ExchangeResult::Reply) {
            const QString name = QString::fromStdString(gnp::identityString(reply)).trimmed();
            if(!name.isEmpty())
                info.name = name;
        }
        CloseHandle(device);

        if(info.percent < 0 || seenNames.contains(info.name))
            continue;
        seenNames.insert(info.name);
        TRC_DEBUG("jabra: pid {:04X} address {:02X} '{}' {}% charging={} via {}", collection.productId, address,
                  info.name.toStdString(), info.percent, info.charging, info.connectionKind.toStdString());
        result.append(info);
    }
    return result;
}
