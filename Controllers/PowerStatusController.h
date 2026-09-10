#pragma once

#include <QObject>
#include <QString>

#include "PowerAlertPolicy.h"

// Watch the system charger. Windows-only.
// A hidden message window receives AC/DC power source notifications.
class PowerStatusController : public QObject {
        Q_OBJECT
        // True while the system runs on the charger.
        Q_PROPERTY(bool pluggedIn READ pluggedIn NOTIFY changed)
        // System battery charge. -1 when unknown.
        Q_PROPERTY(int percent READ percent NOTIFY changed)
        // Charge bucket label of the last reading.
        Q_PROPERTY(QString bucket READ bucket NOTIFY changed)
        // False when the system has no battery, so the UI can hide the feature.
        Q_PROPERTY(bool available READ available NOTIFY availabilityChanged)

    public:
        explicit PowerStatusController(QObject* parent = nullptr);
        ~PowerStatusController() override;

        void start();

        bool pluggedIn() const;
        int percent() const;
        QString bucket() const;
        bool available() const;

        // Read the power state now. Raise an alert when the source changed.
        void refresh();

    signals:
        void changed();
        void availabilityChanged();
        // Raised on plug and unplug only. Never on the first reading.
        void chargerChanged(bool pluggedIn);

    private:
        void setAvailable(bool available);

        mf::PowerAlertPolicy m_policy;
        bool m_pluggedIn = false;
        int m_percent = -1;
        QString m_bucket;
        // Optimistic until a reading proves there is no battery.
        bool m_available = true;
        void* m_hwnd = nullptr;
        void* m_notificationHandle = nullptr;
};
