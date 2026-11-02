#pragma once

#include <QImage>
#include <QQuickImageProvider>
#include <QString>

namespace mf {

// Black modules on white, `quiet` modules of margin. Null image when `text` is empty or too long.
QImage renderQrCode(const QString& text, int modulePx = 8, int quiet = 2);

} // namespace mf

// image://qr/<base64url of the text>. QML sets sourceSize for the pixel size.
class QrImageProvider : public QQuickImageProvider {
    public:
        QrImageProvider();
        QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

        static QString decodeId(const QString& id);
};
