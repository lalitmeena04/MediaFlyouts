#pragma once

#include <QObject>

// Suppresses the native Windows volume OSD so our VolumeFlyout replaces it.
class NativeFlyoutSuppressor : public QObject {
        Q_OBJECT

    public:
        explicit NativeFlyoutSuppressor(QObject* parent = nullptr);
        ~NativeFlyoutSuppressor() override;

        void start();

    public slots:
        // called when our volume changes; hides the native OSD now and briefly after
        void arm();
};
