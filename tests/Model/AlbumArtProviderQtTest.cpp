#include <QtTest>

#include <QImage>

#include "AlbumArtProvider.h"

class AlbumArtProviderQtTest : public QObject {
        Q_OBJECT

    private slots:
        void nullBeforeSet();
        void returnsStoredImage();
        void scalesToRequestedSize();
        void latestImageWins();

    private:
        static QImage solid(int w, int h) {
            QImage img(w, h, QImage::Format_ARGB32);
            img.fill(Qt::red);
            return img;
        }
};

void AlbumArtProviderQtTest::nullBeforeSet() {
    AlbumArtProvider provider;
    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize());
    QVERIFY(result.isNull());
}

void AlbumArtProviderQtTest::returnsStoredImage() {
    AlbumArtProvider provider;
    provider.setImage(solid(200, 100));
    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize());
    QVERIFY(!result.isNull());
    QCOMPARE(result.size(), QSize(200, 100));
    QCOMPARE(size, QSize(200, 100));
}

void AlbumArtProviderQtTest::scalesToRequestedSize() {
    AlbumArtProvider provider;
    provider.setImage(solid(200, 100));
    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize(50, 50));
    QVERIFY(result.width() >= 50);
    QVERIFY(result.height() >= 50);
}

void AlbumArtProviderQtTest::latestImageWins() {
    AlbumArtProvider provider;
    provider.setImage(solid(10, 10));
    provider.setImage(solid(64, 32));
    QSize size;
    const QImage result = provider.requestImage(QStringLiteral("cover"), &size, QSize());
    QCOMPARE(result.size(), QSize(64, 32));
}

QTEST_GUILESS_MAIN(AlbumArtProviderQtTest)
#include "AlbumArtProviderQtTest.moc"
