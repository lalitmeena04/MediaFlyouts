#include "QrCode.h"

#include <qrcodegen.hpp>

#include "Logging.h"

namespace mf {

QImage renderQrCode(const QString& text, int modulePx, int quiet) {
    if(text.isEmpty() || modulePx < 1 || quiet < 0)
        return {};
    using qrcodegen::QrCode;
    try {
        const QrCode qr = QrCode::encodeText(text.toUtf8().constData(), QrCode::Ecc::MEDIUM);
        const int modules = qr.getSize();
        const int side = (modules + 2 * quiet) * modulePx;
        QImage image(side, side, QImage::Format_RGB32);
        image.fill(Qt::white);
        for(int y = 0; y < modules; ++y) {
            for(int x = 0; x < modules; ++x) {
                if(!qr.getModule(x, y))
                    continue;
                const int px = (x + quiet) * modulePx;
                const int py = (y + quiet) * modulePx;
                for(int dy = 0; dy < modulePx; ++dy) {
                    QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(py + dy));
                    for(int dx = 0; dx < modulePx; ++dx)
                        line[px + dx] = qRgb(0, 0, 0);
                }
            }
        }
        return image;
    } catch(const std::exception& e) {
        TRC_WARN("qr: cannot encode {} chars: {}", text.size(), e.what());
        return {};
    }
}

} // namespace mf

QrImageProvider::QrImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

QString QrImageProvider::decodeId(const QString& id) {
    const auto decoded = QByteArray::fromBase64Encoding(
        id.toLatin1(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals | QByteArray::AbortOnBase64DecodingErrors);
    return decoded ? QString::fromUtf8(*decoded) : QString();
}

QImage QrImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    const QString text = decodeId(id);
    const int target = requestedSize.isValid() ? qMax(requestedSize.width(), requestedSize.height()) : 256;
    QImage image = mf::renderQrCode(text, 1, 2);
    if(image.isNull()) {
        if(size)
            *size = QSize();
        return {};
    }
    // Scale whole modules only, so the edges stay crisp.
    const int scale = qMax(1, target / image.width());
    image = image.scaled(image.width() * scale, image.height() * scale, Qt::KeepAspectRatio, Qt::FastTransformation);
    if(size)
        *size = image.size();
    return image;
}
