#include <gtest/gtest.h>

#include <QColor>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QList>
#include <QSize>
#include <QString>

// Checks the files tools/generate_icon.ps1 writes. They feed the exe
// resource, the installer, the MSIX tiles and the in-app logo.
namespace {

QString assetPath(const char* relative) {
    return QStringLiteral(MEDIAFLYOUTS_SOURCE_DIR) + QLatin1Char('/') + QString::fromLatin1(relative);
}

// Every frame size inside a multi-image file such as an .ico.
QList<QSize> frameSizes(const QString& path) {
    QImageReader reader(path);
    QList<QSize> sizes;
    do {
        const QImage frame = reader.read();
        if(frame.isNull())
            break;
        sizes.append(frame.size());
    } while(reader.jumpToNextImage());
    return sizes;
}

bool cornersAreTransparent(const QImage& image) {
    const int r = image.width() - 1;
    const int b = image.height() - 1;
    for(const QPoint& p : {QPoint(0, 0), QPoint(r, 0), QPoint(0, b), QPoint(r, b)}) {
        if(image.pixelColor(p).alpha() != 0)
            return false;
    }
    return true;
}

// True when at least one pixel is fully clear, so the background shows
// through. Tight-cropped frames cannot promise clear corners: at 16-48 px
// the mark's rounded corners are smaller than one pixel.
bool hasClearPixels(const QImage& image) {
    for(int y = 0; y < image.height(); ++y) {
        for(int x = 0; x < image.width(); ++x) {
            if(image.pixelColor(x, y).alpha() == 0)
                return true;
        }
    }
    return false;
}

} // namespace

TEST(IconAssetsTest, MasterLogoIsSquareWithTransparentBackground) {
    const QImage master(assetPath("packaging/assets/media-flyouts.png"));
    ASSERT_FALSE(master.isNull());
    EXPECT_EQ(master.width(), master.height());
    EXPECT_GE(master.width(), 512);
    EXPECT_TRUE(master.hasAlphaChannel());
    EXPECT_TRUE(cornersAreTransparent(master));
}

TEST(IconAssetsTest, IcoHoldsEveryShellSize) {
    const QString ico = assetPath("packaging/assets/MediaFlyouts.ico");
    ASSERT_TRUE(QImageReader::supportedImageFormats().contains("ico")) << "qico plugin missing";

    const QList<QSize> sizes = frameSizes(ico);
    // Tray at 100 %..400 %, taskbar, Explorer large icon.
    for(int side : {16, 20, 24, 32, 48, 64, 128, 256})
        EXPECT_TRUE(sizes.contains(QSize(side, side))) << "missing " << side << " px frame";
}

TEST(IconAssetsTest, IcoFramesCarryAlphaAndFillTheCanvas) {
    QImageReader reader(assetPath("packaging/assets/MediaFlyouts.ico"));
    int frames = 0;
    do {
        const QImage frame = reader.read();
        if(frame.isNull())
            break;
        ++frames;
        EXPECT_TRUE(frame.hasAlphaChannel()) << frame.width() << " px frame has no alpha";
        EXPECT_TRUE(hasClearPixels(frame)) << frame.width() << " px frame is flattened onto a background";
        // Corner pixels are a safe check only once they are smaller than the mark's corner radius.
        if(frame.width() >= 64)
            EXPECT_TRUE(cornersAreTransparent(frame)) << frame.width() << " px frame has an opaque corner";
        // Mark is cropped tight. The middle row must hit ink.
        bool inkOnMiddleRow = false;
        for(int x = 0; x < frame.width() && !inkOnMiddleRow; ++x)
            inkOnMiddleRow = frame.pixelColor(x, frame.height() / 2).alpha() > 200;
        EXPECT_TRUE(inkOnMiddleRow) << frame.width() << " px frame looks empty";
    } while(reader.jumpToNextImage());
    EXPECT_GE(frames, 8);
}

TEST(IconAssetsTest, InAppLogoIs256PxWithAlpha) {
    const QImage logo(assetPath("packaging/assets/icon-256.png"));
    ASSERT_FALSE(logo.isNull());
    EXPECT_EQ(logo.size(), QSize(256, 256));
    EXPECT_TRUE(logo.hasAlphaChannel());
    EXPECT_TRUE(cornersAreTransparent(logo));
}

TEST(IconAssetsTest, MsixTilesMatchTheManifestDimensions) {
    struct Tile {
        const char* file;
        QSize size;
    };
    const Tile tiles[] = {
        {"packaging/msix/Images/StoreLogo.png", QSize(50, 50)},
        {"packaging/msix/Images/Square44x44Logo.png", QSize(44, 44)},
        {"packaging/msix/Images/Square71x71Logo.png", QSize(71, 71)},
        {"packaging/msix/Images/Square150x150Logo.png", QSize(150, 150)},
        {"packaging/msix/Images/Wide310x150Logo.png", QSize(310, 150)},
    };
    for(const Tile& tile : tiles) {
        const QImage image(assetPath(tile.file));
        ASSERT_FALSE(image.isNull()) << tile.file;
        EXPECT_EQ(image.size(), tile.size) << tile.file;
        EXPECT_TRUE(cornersAreTransparent(image)) << tile.file << " should let the tile colour show";
    }
}

TEST(IconAssetsTest, AppBundlesTheIconAndUsesItForTrayAndWindows) {
    QFile cmake(assetPath("CMakeLists.txt"));
    ASSERT_TRUE(cmake.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString build = QString::fromUtf8(cmake.readAll());
    EXPECT_TRUE(build.contains(QStringLiteral("PREFIX \"/icons\"")));
    EXPECT_TRUE(build.contains(QStringLiteral("packaging/assets/MediaFlyouts.ico")));
    EXPECT_TRUE(build.contains(QStringLiteral("packaging/assets/icon-256.png")));
    EXPECT_TRUE(build.contains(QStringLiteral("Views/AppLogo.qml")));

    QFile tray(assetPath("Controllers/TrayController.cpp"));
    ASSERT_TRUE(tray.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString traySource = QString::fromUtf8(tray.readAll());
    EXPECT_TRUE(traySource.contains(QStringLiteral("new QSystemTrayIcon(mf::loadAppIcon()")));
    EXPECT_TRUE(traySource.contains(QStringLiteral("m_tray->setIcon(mf::loadAppIcon())")));

    QFile main(assetPath("MainControllerClass.cpp"));
    ASSERT_TRUE(main.open(QIODevice::ReadOnly | QIODevice::Text));
    EXPECT_TRUE(QString::fromUtf8(main.readAll()).contains(QStringLiteral("setWindowIcon(mf::loadAppIcon())")));
}
