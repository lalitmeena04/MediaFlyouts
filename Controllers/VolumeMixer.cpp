#include "VolumeMixer.h"

#include <QBuffer>
#include <QFileInfo>
#include <QImage>
#include <QVariantMap>

#include <windows.h>

#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <shellapi.h>

namespace {

QString processImagePath(DWORD pid) {
    if(pid == 0)
        return {};
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if(!h)
        return {};
    wchar_t buf[MAX_PATH] = {};
    DWORD size = MAX_PATH;
    QString path;
    if(QueryFullProcessImageNameW(h, 0, buf, &size))
        path = QString::fromWCharArray(buf, size);
    CloseHandle(h);
    return path;
}

// Copies the pixels out of an HICON into an ARGB image, rebuilding the alpha
// channel from the AND mask for legacy icons that ship without one.
QImage imageFromHICON(HICON icon) {
    if(!icon)
        return {};
    ICONINFO ii = {};
    if(!GetIconInfo(icon, &ii))
        return {};
    BITMAP bm = {};
    GetObject(ii.hbmColor, sizeof(bm), &bm);
    const int w = bm.bmWidth;
    const int h = bm.bmHeight;

    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h; // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    QImage img(w, h, QImage::Format_ARGB32);
    HDC dc = GetDC(nullptr);
    GetDIBits(dc, ii.hbmColor, 0, h, img.bits(), &bi, DIB_RGB_COLORS);

    bool hasAlpha = false;
    for(int y = 0; y < h && !hasAlpha; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for(int x = 0; x < w; ++x)
            if(qAlpha(line[x]) != 0) {
                hasAlpha = true;
                break;
            }
    }
    if(!hasAlpha && ii.hbmMask) {
        QImage mask(w, h, QImage::Format_ARGB32);
        GetDIBits(dc, ii.hbmMask, 0, h, mask.bits(), &bi, DIB_RGB_COLORS);
        for(int y = 0; y < h; ++y) {
            QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
            const QRgb* mline = reinterpret_cast<const QRgb*>(mask.constScanLine(y));
            for(int x = 0; x < w; ++x)
                line[x] = qRgba(qRed(line[x]), qGreen(line[x]), qBlue(line[x]), qRed(mline[x]) == 0 ? 255 : 0);
        }
    }
    ReleaseDC(nullptr, dc);
    if(ii.hbmColor)
        DeleteObject(ii.hbmColor);
    if(ii.hbmMask)
        DeleteObject(ii.hbmMask);
    return img;
}

QString iconDataUri(const QString& exePath) {
    if(exePath.isEmpty())
        return {};
    SHFILEINFOW sfi = {};
    if(!SHGetFileInfoW(reinterpret_cast<const wchar_t*>(exePath.utf16()), 0, &sfi, sizeof(sfi),
                       SHGFI_ICON | SHGFI_LARGEICON))
        return {};
    QImage img = imageFromHICON(sfi.hIcon);
    if(sfi.hIcon)
        DestroyIcon(sfi.hIcon);
    if(img.isNull())
        return {};
    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(png.toBase64());
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
        QString iconUri;
        if(control2) {
            if(control2->IsSystemSoundsSession() == S_OK) {
                name = QStringLiteral("System sounds");
            } else {
                DWORD pid = 0;
                control2->GetProcessId(&pid);
                const QString path = processImagePath(pid);
                name = path.isEmpty() ? QStringLiteral("PID %1").arg(pid) : QFileInfo(path).completeBaseName();
                iconUri = iconDataUri(path);
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
            entry[QStringLiteral("icon")] = iconUri;
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
