#pragma once

#include <QObject>

// Parks the native Windows volume OSD host off-screen (gpkgpk/HideVolumeOSD technique) so our
// VolumeFlyout fully replaces it without any flash. Restores it on shutdown.
class NativeFlyoutSuppressor : public QObject {
        Q_OBJECT

    public:
        explicit NativeFlyoutSuppressor(QObject* parent = nullptr);
        ~NativeFlyoutSuppressor() override;

        void start();

    public slots:
        // ensure the native OSD host has been parked off-screen (idempotent)
        void arm();

    private:
        bool parkOsd();
        void restoreOsd();

        void* m_osdHost = nullptr; // HWND, kept as void* so windows.h stays out of the header
        long m_originalExStyle = 0;
        bool m_parked = false;
};
