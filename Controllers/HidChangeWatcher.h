#pragma once

#include <QObject>

// Watch HID devices plug and unplug.
// BatteryStatusController reacts before next poll.
// Windows-only. Uses RegisterDeviceNotification on hidden message window.
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
