#include "LogitechBatteryProvider.h"

#include "HidCollections.h"
#include "HidppProtocol.h"
#include "Logging.h"

#include <QHash>
#include <QSet>

#include <algorithm>
#include <map>
#include <string>

namespace {

constexpr USHORT kLogitechVendorId = 0x046D;
// HID++ over USB and receivers lives on this vendor page.
// Bluetooth devices (page 0xFF43) are left to BluetoothLeBatteryProvider. It already lists them.
constexpr USAGE kUsbVendorPage = 0xFF00;

constexpr DWORD kExchangeTimeoutMs = 1200; // sleeping devices wake slowly
constexpr DWORD kScanQuietMs = 400;        // scan ends this long after the last frame
constexpr DWORD kScanMaxMs = 1500;         // also the wait for the first frame
constexpr int kMaxNameChunks = 3;
// Silent collection is retried after this many polls. Receiver stays silent while its devices sleep.
constexpr int kSilentSkipPolls = 4;

bool isHidppPage(USAGE page) {
    return page == kUsbVendorPage;
}

// Windows splits short and long HID++ reports into two collections.
// Device replies land on the long one. Prefer it when the product has one.
bool hasLongCollection(const std::vector<HidCollection>& collections, USHORT productId) {
    for(const HidCollection& collection : collections) {
        if(collection.productId == productId && isHidppPage(collection.usagePage)
           && collection.outputLength >= hidpp::kLongReportSize)
            return true;
    }
    return false;
}

// Windows requires exact output report length.
std::vector<std::uint8_t> frameFor(const HidCollection& collection, std::vector<std::uint8_t> request) {
    if(collection.outputLength >= hidpp::kLongReportSize)
        request = hidpp::asLongReport(std::move(request));
    request.resize(collection.outputLength, 0);
    return request;
}

bool writeReport(HANDLE device, const std::vector<std::uint8_t>& frame) {
    OVERLAPPED ov = {};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    DWORD written = 0;
    BOOL ok = WriteFile(device, frame.data(), static_cast<DWORD>(frame.size()), &written, &ov);
    if(!ok && GetLastError() == ERROR_IO_PENDING) {
        if(WaitForSingleObject(ov.hEvent, 300) == WAIT_OBJECT_0)
            ok = GetOverlappedResult(device, &ov, &written, FALSE);
        else
            CancelIo(device);
    }
    CloseHandle(ov.hEvent);
    return ok && written == frame.size();
}

// False on timeout. Frame can be a notification or reply to other request.
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

// Write one request. Wait for its reply. Skip unrelated frames.
bool exchange(HANDLE device, const HidCollection& collection, const std::vector<std::uint8_t>& request,
              std::vector<std::uint8_t>& reply) {
    if(!writeReport(device, frameFor(collection, request)))
        return false;
    const ULONGLONG deadline = GetTickCount64() + kExchangeTimeoutMs;
    for(ULONGLONG now = GetTickCount64(); now < deadline; now = GetTickCount64()) {
        if(!readReport(device, collection.inputLength, static_cast<DWORD>(deadline - now), reply))
            return false;
        if(hidpp::isReplyTo(reply, request))
            return true;
    }
    return false;
}

// Ask every device index at once. Replies carry the index.
// Returns index -> UnifiedBattery feature index. 0 means device answered without it.
// Empty receiver slots stay silent on the long collection.
// A direct device echoes every index. Then only 0xFF is kept.
std::map<std::uint8_t, std::uint8_t> scanDevices(HANDLE device, const HidCollection& collection,
                                                 bool& sawHidppFrame) {
    std::map<std::uint8_t, std::vector<std::uint8_t>> requests;
    requests[hidpp::kDirectDeviceIndex] =
        hidpp::buildGetFeatureIndex(hidpp::kDirectDeviceIndex, hidpp::kFeatureUnifiedBattery);
    for(std::uint8_t slot = 0x01; slot <= hidpp::kMaxReceiverSlot; ++slot)
        requests[slot] = hidpp::buildGetFeatureIndex(slot, hidpp::kFeatureUnifiedBattery);
    for(const auto& [index, request] : requests)
        writeReport(device, frameFor(collection, request));

    std::map<std::uint8_t, std::uint8_t> answered;
    const ULONGLONG start = GetTickCount64();
    ULONGLONG lastFrame = 0;
    std::vector<std::uint8_t> frame;
    while(true) {
        const ULONGLONG now = GetTickCount64();
        if(now - start >= kScanMaxMs || (lastFrame != 0 && now - lastFrame >= kScanQuietMs))
            break;
        DWORD remaining = static_cast<DWORD>(kScanMaxMs - (now - start));
        if(lastFrame != 0)
            remaining = static_cast<DWORD>(std::min<ULONGLONG>(remaining, kScanQuietMs - (now - lastFrame)));
        if(!readReport(device, collection.inputLength, remaining, frame))
            continue;
        const int index = hidpp::deviceIndexOf(frame);
        if(index < 0)
            continue;
        sawHidppFrame = true;
        lastFrame = GetTickCount64();
        const auto request = requests.find(static_cast<std::uint8_t>(index));
        if(request != requests.end() && hidpp::isReplyTo(frame, request->second))
            answered[request->first] = hidpp::featureIndexOf(frame);
    }

    const auto direct = answered.find(hidpp::kDirectDeviceIndex);
    if(direct != answered.end())
        return {*direct};
    return answered;
}

std::uint8_t featureIndex(HANDLE device, const HidCollection& collection, std::uint8_t deviceIndex,
                          std::uint16_t featureId) {
    std::vector<std::uint8_t> reply;
    if(!exchange(device, collection, hidpp::buildGetFeatureIndex(deviceIndex, featureId), reply))
        return 0;
    return hidpp::featureIndexOf(reply);
}

// Try unified battery first. Then try legacy battery.
int readBattery(HANDLE device, const HidCollection& collection, std::uint8_t deviceIndex,
                std::uint8_t unifiedIndex, bool& charging) {
    std::vector<std::uint8_t> reply;
    if(unifiedIndex != 0
       && exchange(device, collection, hidpp::buildFunctionCall(deviceIndex, unifiedIndex, 0x01), reply)) {
        charging = hidpp::unifiedBatteryCharging(reply);
        return hidpp::unifiedBatteryPercent(reply);
    }
    if(const std::uint8_t legacyIndex = featureIndex(device, collection, deviceIndex, hidpp::kFeatureBatteryStatus)) {
        if(exchange(device, collection, hidpp::buildFunctionCall(deviceIndex, legacyIndex, 0x00), reply)) {
            charging = hidpp::legacyBatteryCharging(reply);
            return hidpp::legacyBatteryPercent(reply);
        }
    }
    return -1;
}

// Read device name and type through feature 0x0005.
// Receiver product string names the dongle, not the device.
void readIdentity(HANDLE device, const HidCollection& collection, std::uint8_t deviceIndex, QString& name,
                  QString& category) {
    const std::uint8_t nameIndex = featureIndex(device, collection, deviceIndex, hidpp::kFeatureDeviceName);
    if(nameIndex == 0)
        return;
    std::vector<std::uint8_t> reply;
    if(exchange(device, collection, hidpp::buildFunctionCall(deviceIndex, nameIndex, 0x00), reply)) {
        const int length = hidpp::deviceNameLength(reply);
        std::string collected;
        for(int chunk = 0; chunk < kMaxNameChunks && static_cast<int>(collected.size()) < length; ++chunk) {
            const auto offset = static_cast<std::uint8_t>(collected.size());
            if(!exchange(device, collection, hidpp::buildFunctionCall(deviceIndex, nameIndex, 0x01, {offset}),
                         reply))
                break;
            const std::string part = hidpp::deviceNameChunk(reply);
            if(part.empty())
                break;
            collected += part;
        }
        const QString trimmed = QString::fromStdString(collected).trimmed();
        if(!trimmed.isEmpty())
            name = trimmed;
    }
    if(exchange(device, collection, hidpp::buildFunctionCall(deviceIndex, nameIndex, 0x02), reply)) {
        const std::string type = hidpp::deviceTypeCategory(reply);
        if(!type.empty())
            category = QString::fromStdString(type);
    }
}

} // namespace

QVector<BatteryDeviceInfo> LogitechBatteryProvider::poll() {
    QVector<BatteryDeviceInfo> result;
    QSet<QString> seenNames;
    const std::vector<HidCollection> collections = enumerateHidCollections(kLogitechVendorId);

    QSet<QString> paths;
    for(const HidCollection& collection : collections)
        paths.insert(collection.path);
    if(paths != m_lastPaths) {
        m_lastPaths = paths;
        m_skipPolls.clear();
    }

    for(const HidCollection& collection : collections) {
        if(!isHidppPage(collection.usagePage) || collection.outputLength == 0)
            continue;
        if(collection.outputLength < hidpp::kLongReportSize && hasLongCollection(collections, collection.productId))
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
        if(device == INVALID_HANDLE_VALUE)
            continue;

        bool sawHidppFrame = false;
        const std::map<std::uint8_t, std::uint8_t> devices = scanDevices(device, collection, sawHidppFrame);
        if(!sawHidppFrame) {
            TRC_DEBUG("logitech: no HID++ frame from pid {:04X} usage {:04X}/{:04X}, skip {} polls",
                      collection.productId, collection.usagePage, collection.usage, kSilentSkipPolls);
            m_skipPolls.insert(collection.path, kSilentSkipPolls);
        }

        for(const auto& [deviceIndex, unifiedIndex] : devices) {
            bool charging = false;
            const int percent = readBattery(device, collection, deviceIndex, unifiedIndex, charging);
            if(percent < 0)
                continue;

            BatteryDeviceInfo info;
            info.name = collection.product.isEmpty() ? QStringLiteral("Logitech device") : collection.product;
            info.category = categoryForProduct(collections, collection.productId);
            readIdentity(device, collection, deviceIndex, info.name, info.category);
            if(seenNames.contains(info.name))
                info.name = QStringLiteral("%1 (%2)").arg(info.name).arg(deviceIndex);
            if(seenNames.contains(info.name))
                continue;
            seenNames.insert(info.name);

            info.percent = percent;
            info.charging = charging;
            info.connectionKind = QStringLiteral("wireless");
            TRC_DEBUG("logitech: pid {:04X} index {:02X} '{}' {}% charging={} category={}", collection.productId,
                      deviceIndex, info.name.toStdString(), percent, charging, info.category.toStdString());
            result.append(info);
        }
        CloseHandle(device);
    }
    return result;
}
