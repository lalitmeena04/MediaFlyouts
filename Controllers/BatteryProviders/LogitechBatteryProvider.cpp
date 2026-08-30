#include "LogitechBatteryProvider.h"

#include "HidCollections.h"
#include "HidppProtocol.h"

#include <QSet>

namespace {

constexpr USHORT kLogitechVendorId = 0x046D;
constexpr USAGE kVendorUsagePage = 0xFF00;

// Writes a HID++ report and waits for the reply addressed to the same device index.
// Windows requires the write buffer to be exactly the collection's output report length.
bool exchange(HANDLE device, const HidCollection& collection, const std::vector<std::uint8_t>& request,
              std::uint8_t deviceIndex, std::vector<std::uint8_t>& reply) {
    std::vector<std::uint8_t> padded = request;
    padded.resize(collection.outputLength, 0);

    OVERLAPPED writeOv = {};
    writeOv.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    DWORD written = 0;
    if(!WriteFile(device, padded.data(), static_cast<DWORD>(padded.size()), &written, &writeOv)
       && GetLastError() == ERROR_IO_PENDING)
        WaitForSingleObject(writeOv.hEvent, 300);
    CloseHandle(writeOv.hEvent);

    // Kept short: a vendor collection that isn't HID++ (some G headsets) never answers, and
    // this runs for every device index on every poll.
    for(int attempt = 0; attempt < 4; ++attempt) {
        reply.assign(collection.inputLength, 0);
        OVERLAPPED readOv = {};
        readOv.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        DWORD read = 0;
        BOOL ok = ReadFile(device, reply.data(), collection.inputLength, &read, &readOv);
        if(!ok && GetLastError() == ERROR_IO_PENDING) {
            if(WaitForSingleObject(readOv.hEvent, 300) != WAIT_OBJECT_0) {
                CancelIo(device);
                CloseHandle(readOv.hEvent);
                continue;
            }
            GetOverlappedResult(device, &readOv, &read, FALSE);
            ok = TRUE;
        }
        CloseHandle(readOv.hEvent);
        if(ok && read > 0 && hidpp::isReplyFor(reply, deviceIndex))
            return true;
    }
    return false;
}

// Resolves a feature id to this device's index for it; 0 means unsupported.
std::uint8_t featureIndex(HANDLE device, const HidCollection& collection, std::uint8_t deviceIndex,
                          std::uint16_t featureId) {
    std::vector<std::uint8_t> reply;
    if(!exchange(device, collection, hidpp::buildGetFeatureIndex(deviceIndex, featureId), deviceIndex, reply))
        return 0;
    return hidpp::featureIndexOf(reply);
}

// Tries the modern unified battery feature first, then the legacy one. 'answered' reports
// whether the collection spoke HID++ at all, so callers can stop probing dead collections.
int readBattery(HANDLE device, const HidCollection& collection, std::uint8_t deviceIndex, bool& answered) {
    std::vector<std::uint8_t> reply;
    answered = exchange(device, collection,
                        hidpp::buildGetFeatureIndex(deviceIndex, hidpp::kFeatureUnifiedBattery), deviceIndex,
                        reply);
    if(answered) {
        if(const std::uint8_t index = hidpp::featureIndexOf(reply)) {
            if(exchange(device, collection, hidpp::buildFunctionCall(deviceIndex, index, 0x01), deviceIndex,
                        reply))
                return hidpp::unifiedBatteryPercent(reply);
        }
    }
    if(const std::uint8_t index = featureIndex(device, collection, deviceIndex, hidpp::kFeatureBatteryStatus)) {
        answered = true;
        if(exchange(device, collection, hidpp::buildFunctionCall(deviceIndex, index, 0x00), deviceIndex, reply))
            return hidpp::legacyBatteryPercent(reply);
    }
    return -1;
}

} // namespace

QVector<BatteryDeviceInfo> LogitechBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;
    QSet<QString> seenNames;
    const std::vector<HidCollection> collections = enumerateHidCollections(kLogitechVendorId);

    for(const HidCollection& collection : collections) {
        // HID++ lives on the vendor-defined collection
        if(collection.usagePage != kVendorUsagePage || collection.outputLength == 0)
            continue;

        if(m_notHidpp.contains(collection.path))
            continue;

        HANDLE device = CreateFileW(reinterpret_cast<const wchar_t*>(collection.path.utf16()),
                                    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                    OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        if(device == INVALID_HANDLE_VALUE)
            continue;

        // 0xFF is a directly attached device; receivers expose their paired devices in slots
        std::vector<std::uint8_t> indices{hidpp::kDirectDeviceIndex};
        for(std::uint8_t slot = 0x01; slot <= hidpp::kMaxReceiverSlot; ++slot)
            indices.push_back(slot);

        bool collectionSpeaksHidpp = false;
        for(std::size_t position = 0; position < indices.size(); ++position) {
            const std::uint8_t deviceIndex = indices[position];
            bool answered = false;
            const int percent = readBattery(device, collection, deviceIndex, answered);
            collectionSpeaksHidpp = collectionSpeaksHidpp || answered;
            // a collection that ignored both the direct index and the first receiver slot
            // isn't HID++ at all; don't spend a poll walking the remaining slots
            if(!collectionSpeaksHidpp && position >= 1)
                break;
            if(percent < 0)
                continue;

            BatteryDeviceInfo info;            info.name = collection.product.isEmpty() ? QStringLiteral("Logitech device") : collection.product;
            if(deviceIndex != hidpp::kDirectDeviceIndex && indices.size() > 1)
                info.name = QStringLiteral("%1 (%2)").arg(info.name).arg(deviceIndex);
            if(seenNames.contains(info.name))
                continue;
            seenNames.insert(info.name);

            info.percent = percent;
            info.connectionKind = QStringLiteral("wireless");
            info.category = categoryForProduct(collections, collection.productId);
            result.append(info);
        }
        if(!collectionSpeaksHidpp)
            m_notHidpp.insert(collection.path);
        CloseHandle(device);
    }
    return result;
}
