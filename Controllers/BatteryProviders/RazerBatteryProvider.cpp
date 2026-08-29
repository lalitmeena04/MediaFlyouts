#include "RazerBatteryProvider.h"

#include "HidCollections.h"
#include "RazerProtocol.h"

#include <QSet>
#include <QThread>

namespace {

constexpr USHORT kRazerVendorId = 0x1532;

// Transaction ids vary by model; these are the ones seen in the wild, low ids first
// because that is what current wireless mice answer on.
constexpr std::uint8_t kTransactionIds[] = {0x00, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F, 0x9F};

// Sends a power-class command and waits for the matching answer. The device reports
// "busy" until it has processed the request, and also emits unrelated reports in between.
bool exchange(HANDLE device, std::uint8_t transactionId, std::uint8_t commandId,
              std::vector<std::uint8_t>& reply) {
    std::vector<std::uint8_t> request = razer::buildRequest(transactionId, commandId);
    if(!HidD_SetFeature(device, request.data(), static_cast<ULONG>(request.size())))
        return false;

    for(int attempt = 0; attempt < 40; ++attempt) {
        reply.assign(razer::kBufferSize, 0);
        if(!HidD_GetFeature(device, reply.data(), static_cast<ULONG>(reply.size())))
            return false;
        if(razer::isReplyTo(reply, transactionId, commandId) && razer::statusOf(reply) != razer::Status::Busy)
            return true;
        QThread::msleep(15);
    }
    return false;
}

// Walks the known transaction ids until one produces a successful battery answer, trying
// the one that worked last time first.
int readBattery(HANDLE device, std::uint8_t& workingTransactionId, std::uint8_t preferred) {
    std::vector<std::uint8_t> reply;
    std::vector<std::uint8_t> order;
    if(preferred != 0xFF)
        order.push_back(preferred);
    for(std::uint8_t transactionId : kTransactionIds)
        if(transactionId != preferred)
            order.push_back(transactionId);

    for(std::uint8_t transactionId : order) {
        if(!exchange(device, transactionId, razer::kCommandBatteryLevel, reply))
            continue;
        const int percent = razer::batteryPercent(reply, transactionId);
        if(percent >= 0) {
            workingTransactionId = transactionId;
            return percent;
        }
    }
    return -1;
}

} // namespace

QVector<BatteryDeviceInfo> RazerBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;
    QSet<USHORT> handledProducts;
    const std::vector<HidCollection> collections = enumerateHidCollections(kRazerVendorId);

    for(const HidCollection& collection : collections) {
        // the control interface is the one exposing the 90-byte feature report
        if(collection.featureLength != razer::kBufferSize)
            continue;
        if(handledProducts.contains(collection.productId))
            continue;

        // no access rights: Windows refuses read/write on system HID collections, but
        // feature reports work regardless
        HANDLE device = CreateFileW(reinterpret_cast<const wchar_t*>(collection.path.utf16()), 0,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if(device == INVALID_HANDLE_VALUE)
            continue;

        std::uint8_t transactionId = 0;
        const int percent = readBattery(device, transactionId, m_transactionIds.value(collection.productId, 0xFF));
        if(percent < 0) {
            CloseHandle(device);
            continue; // wired device, or the dongle has nothing paired to it
        }
        handledProducts.insert(collection.productId);
        m_transactionIds.insert(collection.productId, transactionId);

        BatteryDeviceInfo info;
        info.name = collection.product.isEmpty() ? QStringLiteral("Razer device") : collection.product;
        info.percent = percent;
        info.connectionKind = QStringLiteral("wireless");
        info.category = categoryForProduct(collections, collection.productId);

        std::vector<std::uint8_t> reply;
        if(exchange(device, transactionId, razer::kCommandChargingStatus, reply)) {
            bool supported = false;
            const bool charging = razer::chargingState(reply, transactionId, supported);
            info.charging = supported && charging;
        }

        CloseHandle(device);
        result.append(info);
    }
    return result;
}
