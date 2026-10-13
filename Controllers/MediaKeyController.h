#pragma once

#include <QObject>

class MediaKeyController : public QObject {
        Q_OBJECT

    public:
        explicit MediaKeyController(QObject* parent = nullptr);
        ~MediaKeyController() override;

        void start();

        bool available() const;

        void notifyPressed();
        void notifyVolumePressed();

    signals:
        void mediaKeyPressed();
        void volumeKeyPressed();
        void availabilityChanged();

    private:
        bool m_available = false;
};
