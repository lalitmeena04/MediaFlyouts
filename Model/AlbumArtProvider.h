#pragma once

#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

class AlbumArtProvider : public QQuickImageProvider {
    public:
        AlbumArtProvider();

        void setImage(const QImage& image);
        QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    private:
        QImage m_image;
        QMutex m_mutex;
};
