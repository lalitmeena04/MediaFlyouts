#pragma once

#include <QObject>

// Watches for HID devices (game controllers, mice, keyboards, ...) being plugged in or
// unplugged, so BatteryStatusController can react immediately instead of waiting for
// the next poll. Windows-only: uses RegisterDeviceNotification on a hidden message window.
class HidChangeWatcher : public QObject {
        Q_OBJECT

    public:
        explicit HidChangeWatcher(QObject* parent = nullptr);
        ~HidChangeWatcher() override;

        void start();

    signals:
        void deviceChanged();

    private:
        void* m_hwnd = nullptr;
        void* m_notificationHandle = nullptr;
};
