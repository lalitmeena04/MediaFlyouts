#include "FastPairBatteryProvider.h"

#include "BatteryUtils.h"
#include "DevNodeHelpers.h"
#include "FastPairProtocol.h"

#include <winsock2.h>
#include <ws2bth.h>
#include <windows.h>

#include <cfgmgr32.h>
#include <bluetoothapis.h>
#include <initguid.h>
#include <devpkey.h>

#include <chrono>
#include <vector>

namespace {

// Google Fast Pair message stream RFCOMM service.
constexpr GUID kFastPairMessageStream = {
    0xdf21fe2c, 0x2515, 0x4fdb, {0x88, 0x86, 0xf1, 0x2c, 0x4d, 0x67, 0x92, 0x7c}};

constexpr wchar_t kFastPairUuidText[] = L"{DF21FE2C-2515-4FDB-8886-F12C4D67927C}";

struct FastPairDevice {
        quint64 address = 0;
        QString name;
        QString category;
};

// PnP service sub-node has generic name. Ask Bluetooth API for paired name.
// Skip devices that are not connected.
bool connectedDeviceInfo(quint64 address, QString& name, QString& category) {
    BLUETOOTH_DEVICE_INFO info = {};
    info.dwSize = sizeof(info);
    info.Address.ullLong = address;
    if(BluetoothGetDeviceInfo(nullptr, &info) != ERROR_SUCCESS || !info.fConnected)
        return false;
    name = QString::fromWCharArray(info.szName);
    category = BatteryUtils::categoryFromClassOfDevice(info.ulClassofDevice);
    return !name.isEmpty();
}

// Device address is 12 hex digits before "_C" in BTHENUM service id.
bool addressFromServiceId(const std::wstring& id, quint64& out) {
    const size_t marker = id.rfind(L"_C");
    if(marker == std::wstring::npos || marker < 12)
        return false;
    const std::wstring hex = id.substr(marker - 12, 12);
    for(wchar_t c : hex)
        if(!iswxdigit(c))
            return false;
    out = std::wcstoull(hex.c_str(), nullptr, 16);
    return true;
}

// Paired devices that advertise Fast Pair message stream.
// Use device BTHENUM node for friendly name.
std::vector<FastPairDevice> enumerateFastPairDevices() {
    ULONG listSize = 0;
    if(CM_Get_Device_ID_List_SizeW(&listSize, L"BTHENUM", CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS
       || listSize == 0)
        return {};

    std::vector<wchar_t> list(listSize);
    if(CM_Get_Device_ID_ListW(L"BTHENUM", list.data(), listSize, CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS)
        return {};

    std::vector<FastPairDevice> devices;

    for(const wchar_t* id = list.data(); *id; id += wcslen(id) + 1) {
        std::wstring text(id);
        for(wchar_t& c : text)
            c = towupper(c);
        if(text.find(kFastPairUuidText) == std::wstring::npos)
            continue;

        quint64 address = 0;
        if(!addressFromServiceId(text, address) || address == 0)
            continue;
        bool seen = false;
        for(const FastPairDevice& existing : devices)
            if(existing.address == address)
                seen = true;
        if(seen)
            continue;

        DEVINST devInst = 0;
        if(CM_Locate_DevNodeW(&devInst, const_cast<DEVINSTID_W>(id), CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
            continue;

        QString name;
        QString category;
        if(!connectedDeviceInfo(address, name, category))
            continue; // Paired but not connected now.

        FastPairDevice device;
        device.address = address;
        device.name = name;
        device.category = category;
        devices.push_back(device);
    }
    return devices;
}

// Blocking connect with bounded wait. Off device cannot stall thread.
SOCKET connectFastPair(quint64 address, int timeoutMs) {
    SOCKET sock = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if(sock == INVALID_SOCKET)
        return INVALID_SOCKET;

    SOCKADDR_BTH sa = {};
    sa.addressFamily = AF_BTH;
    sa.btAddr = address;
    sa.serviceClassId = kFastPairMessageStream;
    sa.port = BT_PORT_ANY; // Resolve RFCOMM channel with SDP.

    u_long nonBlocking = 1;
    ioctlsocket(sock, FIONBIO, &nonBlocking);
    connect(sock, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));

    fd_set writable;
    FD_ZERO(&writable);
    FD_SET(sock, &writable);
    timeval tv = {timeoutMs / 1000, (timeoutMs % 1000) * 1000};
    const int ready = select(0, nullptr, &writable, nullptr, &tv);

    u_long blocking = 0;
    ioctlsocket(sock, FIONBIO, &blocking);

    if(ready != 1) {
        closesocket(sock);
        return INVALID_SOCKET;
    }
    return sock;
}

QVector<BatteryComponentInfo> describeComponents(const std::vector<fastpair::BatteryComponent>& components) {
    QVector<BatteryComponentInfo> described;
    if(components.size() < 2)
        return described; // Single-battery device has headline percent.

    // Three components mean true wireless set. Other counts use position labels.
    static const char* twsLabels[] = {"L", "R", "Case"};
    for(size_t i = 0; i < components.size(); ++i) {
        if(components[i].percent < 0)
            continue;
        BatteryComponentInfo info;
        info.label = (components.size() == 3) ? QString::fromLatin1(twsLabels[i]) : QStringLiteral("#%1").arg(i + 1);
        info.percent = components[i].percent;
        info.charging = components[i].charging;
        described.push_back(info);
    }
    return described;
}

} // namespace

FastPairBatteryProvider::FastPairBatteryProvider() {
    WSADATA wsa = {};
    WSAStartup(MAKEWORD(2, 2), &wsa);
    m_thread = std::thread([this] { run(); });
}

FastPairBatteryProvider::~FastPairBatteryProvider() {
    m_stop = true;
    if(m_thread.joinable())
        m_thread.join();
    WSACleanup();
}

QVector<BatteryDeviceInfo> FastPairBatteryProvider::poll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    QVector<BatteryDeviceInfo> devices;
    for(auto it = m_latest.begin(); it != m_latest.end();) {
        // Do not report disconnected device before worker sees socket close.
        if(!bluetoothAddressConnected(it.key())) {
            it = m_latest.erase(it);
            continue;
        }
        devices.push_back(it.value());
        ++it;
    }
    return devices;
}

void FastPairBatteryProvider::run() {
    struct Connection {
            SOCKET sock = INVALID_SOCKET;
            quint64 address = 0;
            QString name;
            QString category;
            std::vector<std::uint8_t> buffer;
    };
    std::vector<Connection> connections;
    auto nextScan = std::chrono::steady_clock::now();
    auto nextLivenessCheck = std::chrono::steady_clock::now();

    auto dropConnection = [&](size_t index) {
        closesocket(connections[index].sock);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_latest.remove(connections[index].address);
        }
        connections.erase(connections.begin() + static_cast<std::ptrdiff_t>(index));
    };

    while(!m_stop) {
        // Disconnect does not always break socket. Poll link state too.
        if(std::chrono::steady_clock::now() >= nextLivenessCheck) {
            nextLivenessCheck = std::chrono::steady_clock::now() + std::chrono::seconds(1);
            for(size_t i = connections.size(); i-- > 0;)
                if(!bluetoothAddressConnected(connections[i].address))
                    dropConnection(i);
        }
        // Periodically pick up newly connected devices.
        if(std::chrono::steady_clock::now() >= nextScan) {
            nextScan = std::chrono::steady_clock::now() + std::chrono::seconds(3);
            for(const FastPairDevice& device : enumerateFastPairDevices()) {
                if(m_stop)
                    break;
                bool connected = false;
                for(const Connection& existing : connections)
                    if(existing.address == device.address)
                        connected = true;
                if(connected)
                    continue;

                const SOCKET sock = connectFastPair(device.address, 3000);
                if(sock == INVALID_SOCKET)
                    continue; // device is paired but not in range/connected
                Connection connection;
                connection.sock = sock;
                connection.address = device.address;
                connection.name = device.name;
                connection.category = device.category;
                connections.push_back(std::move(connection));
            }
        }

        if(connections.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        fd_set readable;
        FD_ZERO(&readable);
        for(const Connection& connection : connections)
            FD_SET(connection.sock, &readable);
        timeval tv = {0, 500 * 1000};
        if(select(0, &readable, nullptr, nullptr, &tv) <= 0)
            continue;

        for(size_t i = connections.size(); i-- > 0;) {
            if(!FD_ISSET(connections[i].sock, &readable))
                continue;

            unsigned char chunk[512];
            const int received = recv(connections[i].sock, reinterpret_cast<char*>(chunk), sizeof(chunk), 0);
            if(received <= 0) {
                dropConnection(i);
                continue;
            }

            Connection& connection = connections[i];
            connection.buffer.insert(connection.buffer.end(), chunk, chunk + received);
            for(const fastpair::Frame& frame : fastpair::drainFrames(connection.buffer)) {
                if(!fastpair::isBatteryFrame(frame))
                    continue;
                const auto components = fastpair::parseBattery(frame);
                const int percent = fastpair::lowestPercent(components);
                if(percent < 0)
                    continue;

                BatteryDeviceInfo info;
                info.name = connection.name;
                info.percent = percent;
                info.connectionKind = QStringLiteral("bluetooth");
                info.category = connection.category;
                info.charging = fastpair::anyCharging(components);
                info.components = describeComponents(components);

                std::lock_guard<std::mutex> lock(m_mutex);
                m_latest.insert(connection.address, info);
            }
        }
    }

    for(Connection& connection : connections)
        closesocket(connection.sock);
}
