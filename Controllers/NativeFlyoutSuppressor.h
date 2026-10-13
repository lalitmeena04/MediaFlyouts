#pragma once

#include <QObject>

class NativeFlyoutSuppressor : public QObject {
        Q_OBJECT

    public:
        explicit NativeFlyoutSuppressor(QObject* parent = nullptr);
        ~NativeFlyoutSuppressor() override;

        void start();

    public slots:
        void arm();
        void restore();

    private:
        bool parkOsd();
        void restoreOsd();

        void* m_osdHost = nullptr; // HWND. Keep windows.h out of header.
        long m_originalExStyle = 0;
        bool m_parked = false;
};
