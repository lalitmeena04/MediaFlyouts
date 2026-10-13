#pragma once

#include <QObject>

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
