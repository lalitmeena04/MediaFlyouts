#include <gtest/gtest.h>

#include <QImage>
#include <QString>

#include "QrCode.h"

namespace {
int darkCount(const QImage& image) {
    int dark = 0;
    for(int y = 0; y < image.height(); ++y)
        for(int x = 0; x < image.width(); ++x)
            dark += qRed(image.pixel(x, y)) < 128;
    return dark;
}
} // namespace

TEST(QrCodeTest, RendersASquareWithQuietZone) {
    const QImage image = mf::renderQrCode(QStringLiteral("upi://pay?pa=dev@upi&cu=INR"), 4, 2);
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.width(), image.height());
    // Version 1 is 21 modules. Anything larger is fine, but it must be a module multiple.
    EXPECT_GE(image.width(), (21 + 4) * 4);
    EXPECT_EQ(image.width() % 4, 0);
    // The quiet zone is white.
    EXPECT_EQ(qRed(image.pixel(0, 0)), 255);
    EXPECT_EQ(qRed(image.pixel(image.width() - 1, image.height() - 1)), 255);
    // The top-left finder pattern starts dark right after the quiet zone.
    EXPECT_LT(qRed(image.pixel(2 * 4, 2 * 4)), 128);
    EXPECT_GT(darkCount(image), 0);
}

TEST(QrCodeTest, DifferentTextGivesADifferentCode) {
    const QImage a = mf::renderQrCode(QStringLiteral("alpha"), 1, 0);
    const QImage b = mf::renderQrCode(QStringLiteral("bravo"), 1, 0);
    ASSERT_FALSE(a.isNull());
    ASSERT_FALSE(b.isNull());
    EXPECT_NE(a, b);
}

TEST(QrCodeTest, RejectsBadInput) {
    EXPECT_TRUE(mf::renderQrCode(QString(), 4, 2).isNull());
    EXPECT_TRUE(mf::renderQrCode(QStringLiteral("x"), 0, 2).isNull());
    EXPECT_TRUE(mf::renderQrCode(QStringLiteral("x"), 4, -1).isNull());
    EXPECT_TRUE(mf::renderQrCode(QString(5000, QLatin1Char('a')), 1, 0).isNull());
}

TEST(QrCodeTest, ProviderDecodesBase64UrlIdsAndScalesWholeModules) {
    QrImageProvider provider;
    const QString text = QStringLiteral("upi://pay?pa=dev@upi&pn=Dev%20Name&cu=INR");
    const QString id = QString::fromLatin1(
        text.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    EXPECT_EQ(QrImageProvider::decodeId(id), text);

    QSize size;
    const QImage image = provider.requestImage(id, &size, QSize(148, 148));
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(size, image.size());
    EXPECT_LE(image.width(), 148);
    const QImage reference = mf::renderQrCode(text, 1, 2);
    EXPECT_EQ(image.width() % reference.width(), 0);
}

TEST(QrCodeTest, ProviderReturnsNullForGarbage) {
    QrImageProvider provider;
    QSize size(1, 1);
    EXPECT_TRUE(provider.requestImage(QStringLiteral("***"), &size, QSize(64, 64)).isNull());
    EXPECT_FALSE(size.isValid());
    EXPECT_TRUE(provider.requestImage(QString(), &size, QSize(64, 64)).isNull());
}
