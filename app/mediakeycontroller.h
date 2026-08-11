#pragma once

#include <QObject>

// Watches hardware media keys (play/pause, next, prev, stop) so we can tell a manual skip from an auto-advance.
class MediaKeyController : public QObject {
        Q_OBJECT

    public:
        explicit MediaKeyController(QObject* parent = nullptr);
        ~MediaKeyController() override;

        void start();

        // internal: invoked by the keyboard hook on the main thread
        void notifyPressed();

    signals:
        void mediaKeyPressed();
};
