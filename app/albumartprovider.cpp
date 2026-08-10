#include "albumartprovider.h"

#include <QMutexLocker>

AlbumArtProvider::AlbumArtProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

void AlbumArtProvider::setImage(const QImage& image) {
    QMutexLocker lock(&m_mutex);
    m_image = image;
}

QImage AlbumArtProvider::requestImage(const QString&, QSize* size, const QSize& requestedSize) {
    QMutexLocker lock(&m_mutex);
    QImage image = m_image;
    if(size)
        *size = image.size();
    if(!image.isNull() && !requestedSize.isEmpty())
        image = image.scaled(requestedSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return image;
}
