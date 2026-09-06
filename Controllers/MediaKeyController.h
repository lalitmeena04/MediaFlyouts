#pragma once

#include <QObject>

// Watch hardware media keys. Detect manual skip.
class MediaKeyController : public QObject {
        Q_OBJECT

    public:
        explicit MediaKeyController(QObject* parent = nullptr);
        ~MediaKeyController() override;

        void start();

        // True after low-level keyboard hook installs.
        bool available() const;

        // Internal. Called by keyboard hook on main thread.
        void notifyPressed();
        void notifyVolumePressed();

    signals:
        void mediaKeyPressed();
        void volumeKeyPressed();
        void availabilityChanged();

    private:
        bool m_available = false;
};
