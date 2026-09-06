#pragma once

#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>

// Thread-safe holder for current album art. QML reads image://albumart.
class AlbumArtProvider : public QQuickImageProvider {
    public:
        AlbumArtProvider();

        void setImage(const QImage& image);
        QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    private:
        QImage m_image;
        QMutex m_mutex;
};
