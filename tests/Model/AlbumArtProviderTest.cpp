#include <gtest/gtest.h>

#include <QImage>

#include "AlbumArtProvider.h"

namespace {
QImage solidImage(int w, int h) {
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(Qt::red);
    return img;
}
} // namespace

TEST(AlbumArtProviderTest, ReturnsNullBeforeAnyImageIsSet) {
    AlbumArtProvider provider;
    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize());
    EXPECT_TRUE(result.isNull());
}

TEST(AlbumArtProviderTest, ReturnsStoredImage) {
    AlbumArtProvider provider;
    provider.setImage(solidImage(200, 100));

    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize());
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), QSize(200, 100));
    EXPECT_EQ(size, QSize(200, 100));
}

TEST(AlbumArtProviderTest, ScalesToRequestedSize) {
    AlbumArtProvider provider;
    provider.setImage(solidImage(200, 100));

    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize(50, 50));
    // KeepAspectRatioByExpanding guarantees the result covers the requested box
    EXPECT_GE(result.width(), 50);
    EXPECT_GE(result.height(), 50);
}

TEST(AlbumArtProviderTest, LatestImageWins) {
    AlbumArtProvider provider;
    provider.setImage(solidImage(10, 10));
    provider.setImage(solidImage(64, 32));

    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize());
    EXPECT_EQ(result.size(), QSize(64, 32));
}
