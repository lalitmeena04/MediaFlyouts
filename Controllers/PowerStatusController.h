#pragma once

#include <QObject>
#include <QString>

#include "PowerAlertPolicy.h"

class PowerStatusController : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool pluggedIn READ pluggedIn NOTIFY changed)
        Q_PROPERTY(int percent READ percent NOTIFY changed)
        Q_PROPERTY(QString bucket READ bucket NOTIFY changed)
        Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)

    public:
        explicit PowerStatusController(QObject* parent = nullptr);
        ~PowerStatusController() override;

        void start();

        bool pluggedIn() const;
        int percent() const;
        QString bucket() const;
        bool available() const;

        void refresh();

    signals:
        void changed();
        void availabilityChanged();
        void chargerChanged(bool pluggedIn);

    private:
        void setAvailable(bool available);

        mf::PowerAlertPolicy m_policy;
        bool m_pluggedIn = false;
        int m_percent = -1;
        QString m_bucket;
        bool m_available = true;
        void* m_hwnd = nullptr;
        void* m_notificationHandle = nullptr;
};
