#include "VolumeController.h"

#include <cmath>

#include <QMetaObject>

#include <windows.h>

#include <endpointvolume.h>
#include <mmdeviceapi.h>

namespace {

// our own write context (reserved for future filtering of self-initiated changes)
const GUID kVolumeEventContext = {0x9a1b2c3d, 0x1234, 0x5678, {0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56, 0x78}};

} // namespace

// classic-COM sink that forwards endpoint volume notifications onto the Qt thread
struct VolumeCallback : IAudioEndpointVolumeCallback {
        LONG ref = 1;
        VolumeController* owner = nullptr;

        explicit VolumeCallback(VolumeController* o) : owner(o) {}

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if(riid == __uuidof(IUnknown) || riid == __uuidof(IAudioEndpointVolumeCallback)) {
                *ppv = static_cast<IAudioEndpointVolumeCallback*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override {
            return InterlockedIncrement(&ref);
        }

        ULONG STDMETHODCALLTYPE Release() override {
            const LONG r = InterlockedDecrement(&ref);
            if(r == 0)
                delete this;
            return r;
        }

        HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA data) override {
            const int percent = static_cast<int>(std::lround(data->fMasterVolume * 100.0f));
            const bool muted = data->bMuted != FALSE;
            VolumeController* o = owner;
            QMetaObject::invokeMethod(
                o, [o, percent, muted] { o->applyState(percent, muted, true); }, Qt::QueuedConnection);
            return S_OK;
        }
};

// classic-COM sink that re-binds us to the new default output when the user switches devices
struct DeviceNotificationClient : IMMNotificationClient {
        LONG ref = 1;
        VolumeController* owner = nullptr;

        explicit DeviceNotificationClient(VolumeController* o) : owner(o) {}

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if(riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient)) {
                *ppv = static_cast<IMMNotificationClient*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override {
            return InterlockedIncrement(&ref);
        }
        ULONG STDMETHODCALLTYPE Release() override {
            const LONG r = InterlockedDecrement(&ref);
            if(r == 0)
                delete this;
            return r;
        }

        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR) override {
            // only the console render endpoint drives the flyout
            if(flow == eRender && role == eConsole) {
                VolumeController* o = owner;
                QMetaObject::invokeMethod(o, [o] { o->rebindDefaultDevice(); }, Qt::QueuedConnection);
            }
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) override {
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR) override {
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR) override {
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override {
            return S_OK;
        }
};

VolumeController::VolumeController(QObject* parent) : QObject(parent) {}

VolumeController::~VolumeController() {
    if(m_enumerator && m_deviceNotifier)
        m_enumerator->UnregisterEndpointNotificationCallback(m_deviceNotifier);
    if(m_deviceNotifier)
        m_deviceNotifier->Release();
    releaseEndpoint();
    if(m_enumerator)
        m_enumerator->Release();
}

void VolumeController::releaseEndpoint() {
    if(m_endpoint && m_callback)
        m_endpoint->UnregisterControlChangeNotify(m_callback);
    if(m_callback) {
        m_callback->Release();
        m_callback = nullptr;
    }
    if(m_endpoint) {
        m_endpoint->Release();
        m_endpoint = nullptr;
        emit availabilityChanged();
    }
}

bool VolumeController::acquireEndpoint() {
    if(!m_enumerator)
        return false;

    IMMDevice* device = nullptr;
    if(FAILED(m_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device)) || !device)
        return false;

    HRESULT hr =
        device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&m_endpoint));
    device->Release();
    if(FAILED(hr) || !m_endpoint)
        return false;

    m_callback = new VolumeCallback(this);
    m_endpoint->RegisterControlChangeNotify(m_callback);

    float level = 0.0f;
    BOOL muted = FALSE;
    m_endpoint->GetMasterVolumeLevelScalar(&level);
    m_endpoint->GetMute(&muted);
    applyState(static_cast<int>(std::lround(level * 100.0f)), muted != FALSE, false);
    emit availabilityChanged();
    return true;
}

void VolumeController::rebindDefaultDevice() {
    releaseEndpoint();
    acquireEndpoint();
}

void VolumeController::start() {
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                  reinterpret_cast<void**>(&m_enumerator));
    if(FAILED(hr) || !m_enumerator)
        return;

    // follow the default output device so the flyout always reflects the current one
    m_deviceNotifier = new DeviceNotificationClient(this);
    m_enumerator->RegisterEndpointNotificationCallback(m_deviceNotifier);

    acquireEndpoint();
}

int VolumeController::volume() const {
    return m_volume;
}

bool VolumeController::available() const {
    return m_endpoint != nullptr;
}

bool VolumeController::muted() const {
    return m_muted;
}

void VolumeController::setVolume(int percent) {
    if(!m_endpoint)
        return;
    if(percent < 0)
        percent = 0;
    if(percent > 100)
        percent = 100;
    m_endpoint->SetMasterVolumeLevelScalar(percent / 100.0f, &kVolumeEventContext);
    if(m_muted && percent > 0)
        m_endpoint->SetMute(FALSE, &kVolumeEventContext);
}

void VolumeController::toggleMute() {
    if(!m_endpoint)
        return;
    m_endpoint->SetMute(m_muted ? FALSE : TRUE, &kVolumeEventContext);
}

void VolumeController::handleVolumeKey() {
    if(!m_endpoint)
        return;
    float level = 0.0f;
    BOOL muted = FALSE;
    m_endpoint->GetMasterVolumeLevelScalar(&level);
    m_endpoint->GetMute(&muted);
    applyState(static_cast<int>(std::lround(level * 100.0f)), muted != FALSE, true);
}

void VolumeController::applyState(int percent, bool muted, bool external) {
    const bool valueChanged = (percent != m_volume) || (muted != m_muted);
    m_volume = percent;
    m_muted = muted;
    if(valueChanged)
        emit changed();
    if(external)
        emit volumeChangedExternally();
}
