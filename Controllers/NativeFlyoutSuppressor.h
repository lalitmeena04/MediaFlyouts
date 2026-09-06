#pragma once

#include <QObject>

// Move Windows volume OSD off-screen. Let VolumeFlyout replace it.
// Restore it on shutdown.
class NativeFlyoutSuppressor : public QObject {
        Q_OBJECT

    public:
        explicit NativeFlyoutSuppressor(QObject* parent = nullptr);
        ~NativeFlyoutSuppressor() override;

        void start();

    public slots:
        // Ensure native OSD host is off-screen. Safe to call again.
        void arm();
        // Restore native OSD when override is off.
        void restore();

    private:
        bool parkOsd();
        void restoreOsd();

        void* m_osdHost = nullptr; // HWND. Keep windows.h out of header.
        long m_originalExStyle = 0;
        bool m_parked = false;
};
