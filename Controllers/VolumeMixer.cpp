#include "VolumeMixer.h"

#include <QFileInfo>
#include <QVariantMap>

#include <windows.h>

#include <audiopolicy.h>
#include <mmdeviceapi.h>

namespace {

QString processName(DWORD pid) {
    if(pid == 0)
        return QStringLiteral("System sounds");
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if(!h)
        return QStringLiteral("PID %1").arg(pid);
    wchar_t buf[MAX_PATH] = {};
    DWORD size = MAX_PATH;
    QString name;
    if(QueryFullProcessImageNameW(h, 0, buf, &size))
        name = QFileInfo(QString::fromWCharArray(buf, size)).completeBaseName();
    CloseHandle(h);
    return name.isEmpty() ? QStringLiteral("PID %1").arg(pid) : name;
}

} // namespace

VolumeMixer::VolumeMixer(QObject* parent) : QObject(parent) {}

VolumeMixer::~VolumeMixer() {
    clear();
}

void VolumeMixer::clear() {
    for(Entry& e : m_entries)
        if(e.volume)
            e.volume->Release();
    m_entries.clear();
}

QVariantList VolumeMixer::sessions() const {
    return m_sessions;
}

void VolumeMixer::refresh() {
    clear();
    m_sessions.clear();

    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDevice* device = nullptr;
    IAudioSessionManager2* manager = nullptr;
    IAudioSessionEnumerator* sessions = nullptr;

    auto cleanup = [&] {
        if(sessions)
            sessions->Release();
        if(manager)
            manager->Release();
        if(device)
            device->Release();
        if(enumerator)
            enumerator->Release();
    };

    if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                               reinterpret_cast<void**>(&enumerator)))) {
        emit sessionsChanged();
        return;
    }
    if(FAILED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        cleanup();
        emit sessionsChanged();
        return;
    }
    if(FAILED(device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr,
                               reinterpret_cast<void**>(&manager)))) {
        cleanup();
        emit sessionsChanged();
        return;
    }
    if(FAILED(manager->GetSessionEnumerator(&sessions))) {
        cleanup();
        emit sessionsChanged();
        return;
    }

    int count = 0;
    sessions->GetCount(&count);
    for(int i = 0; i < count; ++i) {
        IAudioSessionControl* control = nullptr;
        if(FAILED(sessions->GetSession(i, &control)) || !control)
            continue;

        IAudioSessionControl2* control2 = nullptr;
        ISimpleAudioVolume* volume = nullptr;
        control->QueryInterface(__uuidof(IAudioSessionControl2), reinterpret_cast<void**>(&control2));
        control->QueryInterface(__uuidof(ISimpleAudioVolume), reinterpret_cast<void**>(&volume));

        // skip inactive sessions so the list stays to what's actually making sound
        AudioSessionState state = AudioSessionStateInactive;
        control->GetState(&state);

        QString name;
        if(control2) {
            if(control2->IsSystemSoundsSession() == S_OK) {
                name = QStringLiteral("System sounds");
            } else {
                DWORD pid = 0;
                control2->GetProcessId(&pid);
                name = processName(pid);
            }
        }

        if(volume && state != AudioSessionStateExpired && !name.isEmpty()) {
            float level = 1.0f;
            BOOL muted = FALSE;
            volume->GetMasterVolume(&level);
            volume->GetMute(&muted);

            QVariantMap entry;
            entry[QStringLiteral("index")] = static_cast<int>(m_entries.size());
            entry[QStringLiteral("name")] = name;
            entry[QStringLiteral("volume")] = qRound(level * 100.0f);
            entry[QStringLiteral("muted")] = muted == TRUE;
            m_sessions.append(entry);

            volume->AddRef();
            m_entries.push_back({volume, name});
        }

        if(volume)
            volume->Release();
        if(control2)
            control2->Release();
        control->Release();
    }

    cleanup();
    emit sessionsChanged();
}

void VolumeMixer::setSessionVolume(int index, int volume) {
    if(index < 0 || index >= static_cast<int>(m_entries.size()) || !m_entries[index].volume)
        return;
    const float level = qBound(0, volume, 100) / 100.0f;
    m_entries[index].volume->SetMasterVolume(level, nullptr);
    if(index < m_sessions.size()) {
        QVariantMap e = m_sessions[index].toMap();
        e[QStringLiteral("volume")] = qBound(0, volume, 100);
        m_sessions[index] = e;
    }
}

void VolumeMixer::setSessionMute(int index, bool muted) {
    if(index < 0 || index >= static_cast<int>(m_entries.size()) || !m_entries[index].volume)
        return;
    m_entries[index].volume->SetMute(muted ? TRUE : FALSE, nullptr);
    if(index < m_sessions.size()) {
        QVariantMap e = m_sessions[index].toMap();
        e[QStringLiteral("muted")] = muted;
        m_sessions[index] = e;
    }
}
