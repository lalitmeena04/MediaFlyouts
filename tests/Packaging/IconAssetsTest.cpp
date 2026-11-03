#include <gtest/gtest.h>

#include <QColor>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QList>
#include <QSize>
#include <QString>

#include <iterator>

#include "AppIcon.h"
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

// Masters may carry export noise (alpha 1..8) that generate_icon.ps1 ignores
// when it crops; rendered assets must be fully clear.
bool cornersAreTransparent(const QImage& image, int alphaThreshold = 0) {
    const int r = image.width() - 1;
    const int b = image.height() - 1;
    for(const QPoint& p : {QPoint(0, 0), QPoint(r, 0), QPoint(0, b), QPoint(r, b)}) {
        if(image.pixelColor(p).alpha() > alphaThreshold)
            return false;
    }
    return true;
}
constexpr int kMasterAlphaNoise = 8;

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
    const QImage master(assetPath("packaging/assets/MediaFlyouts.png"));
    ASSERT_FALSE(master.isNull());
    EXPECT_EQ(master.width(), master.height());
    EXPECT_GE(master.width(), 512);
    EXPECT_TRUE(master.hasAlphaChannel());
    EXPECT_TRUE(cornersAreTransparent(master, kMasterAlphaNoise));
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

// Every Pro variant must be a drop-in for the base icon: same frames, same logo size.
TEST(IconAssetsTest, ProVariantsMirrorTheBaseIcon) {
    struct Variant {
        const char* master;
        const char* ico;
        const char* logo;
    };
    const Variant variants[] = {
        {"packaging/assets/MediaFlyoutsProBar.png", "packaging/assets/MediaFlyoutsProBar.ico",
         "packaging/assets/icon-pro-bar-256.png"},
        {"packaging/assets/MediaFlyoutsProRibbon.png", "packaging/assets/MediaFlyoutsProRibbon.ico",
         "packaging/assets/icon-pro-ribbon-256.png"},
    };
    const QImage base(assetPath("packaging/assets/icon-256.png"));
    for(const Variant& v : variants) {
        const QImage master(assetPath(v.master));
        ASSERT_FALSE(master.isNull()) << v.master;
        EXPECT_EQ(master.width(), master.height()) << v.master;
        EXPECT_GE(master.width(), 512) << v.master;
        EXPECT_TRUE(cornersAreTransparent(master, kMasterAlphaNoise)) << v.master;

        const QList<QSize> sizes = frameSizes(assetPath(v.ico));
        for(int side : {16, 20, 24, 32, 48, 64, 128, 256})
            EXPECT_TRUE(sizes.contains(QSize(side, side))) << v.ico << " missing " << side << " px frame";

        const QImage logo(assetPath(v.logo));
        ASSERT_FALSE(logo.isNull()) << v.logo;
        EXPECT_EQ(logo.size(), QSize(256, 256)) << v.logo;
        EXPECT_TRUE(logo.hasAlphaChannel()) << v.logo;
        EXPECT_TRUE(cornersAreTransparent(logo)) << v.logo;
        EXPECT_NE(logo, base) << v.logo;
    }
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
    for(const mf::ProIconStyle& style : mf::kProIconStyles) {
        // ":/icons/X" -> "packaging/assets/X"
        const QString ico = QStringLiteral("packaging/assets/") + style.icon.mid(QStringLiteral(":/icons/").size());
        const QString logo = QStringLiteral("packaging/assets/") + style.logo.mid(QStringLiteral("qrc:/icons/").size());
        EXPECT_TRUE(build.contains(ico)) << style.id;
        EXPECT_TRUE(build.contains(logo)) << style.id;
        EXPECT_TRUE(QFile::exists(assetPath(ico.toUtf8().constData()))) << style.id;
        EXPECT_TRUE(QFile::exists(assetPath(logo.toUtf8().constData()))) << style.id;
    }
    EXPECT_TRUE(build.contains(QStringLiteral("Views/AppLogo.qml")));

    QFile tray(assetPath("Controllers/TrayController.cpp"));
    ASSERT_TRUE(tray.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString traySource = QString::fromUtf8(tray.readAll());
    EXPECT_TRUE(traySource.contains(QStringLiteral("new QSystemTrayIcon(mf::loadAppIcon(m_iconResource)")));
    EXPECT_TRUE(traySource.contains(QStringLiteral("m_tray->setIcon(mf::loadAppIcon(m_iconResource))")));

    QFile main(assetPath("MainControllerClass.cpp"));
    ASSERT_TRUE(main.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString mainSource = QString::fromUtf8(main.readAll());
    EXPECT_TRUE(mainSource.contains(QStringLiteral("setWindowIcon(mf::loadAppIcon(m_appIconResource))")));
    // Licence and style can change at runtime; tray and window frames follow both.
    EXPECT_TRUE(mainSource.contains(
        QStringLiteral("mf::appIconResource(m_entitlement->licensed(), m_settings->proIconStyle())")));
    EXPECT_TRUE(mainSource.contains(QStringLiteral("m_tray->setIconResource(resource)")));
    EXPECT_TRUE(mainSource.contains(QStringLiteral("&Settings::changed, m_tray.get(), applyAppIcon")));
}

TEST(IconAssetsTest, AppIconResourceSwitchesOnLicenceAndStyle) {
    EXPECT_EQ(mf::appIconResource(false), mf::kAppIconResource);
    EXPECT_EQ(mf::appIconResource(false, 1), mf::kAppIconResource);
    EXPECT_EQ(mf::appIconResource(true), mf::kProIconStyles[0].icon);
    EXPECT_EQ(mf::appIconResource(true, 1), mf::kProIconStyles[1].icon);
    // Out of range falls back to the default style, never to a missing file.
    EXPECT_EQ(mf::appIconResource(true, -1), mf::kProIconStyles[0].icon);
    EXPECT_EQ(mf::appIconResource(true, 99), mf::kProIconStyles[0].icon);
    EXPECT_EQ(static_cast<int>(std::size(mf::kProIconStyles)), mf::kProIconStyleCount);
    EXPECT_STREQ(mf::kProIconStyles[0].id, "bar");
}
