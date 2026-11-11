#include "BrowserArtworkImage.h"

#include <QDir>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QIcon>
#include <QSettings>
#include <QStandardPaths>

#include "Logging.h"

#ifdef Q_OS_WIN
#include <objbase.h>
#include <windows.h>
#endif

namespace mf {

QStringList firefoxAppUserModelIds() {
    QStringList ids;
#ifdef Q_OS_WIN
    QStringList executables;
    const QString searchResult = QStandardPaths::findExecutable(QStringLiteral("firefox.exe"));
    if(!searchResult.isEmpty())
        executables.append(searchResult);
    const QString appPaths = QStringLiteral("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\firefox.exe");
    for(const QString& root : {QStringLiteral("HKEY_CURRENT_USER\\"), QStringLiteral("HKEY_LOCAL_MACHINE\\")}) {
        QSettings registry(root + appPaths, QSettings::NativeFormat);
        const QString path = registry.value(QStringLiteral("Default")).toString();
        if(QFileInfo(path).isFile() && !executables.contains(path, Qt::CaseInsensitive))
            executables.append(path);
    }
    for(const QString& path : executables) {
        const QString directory = QDir::toNativeSeparators(QFileInfo(path).absolutePath());
        for(HKEY root : {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE}) {
            wchar_t value[256] = {};
            DWORD size = sizeof(value);
            if(RegGetValueW(root, L"Software\\Mozilla\\Firefox\\TaskBarIDs",
                            reinterpret_cast<LPCWSTR>(directory.utf16()), RRF_RT_REG_SZ, nullptr, value, &size) == ERROR_SUCCESS) {
                const QString id = QString::fromWCharArray(value);
                if(!id.isEmpty() && !ids.contains(id, Qt::CaseInsensitive))
                    ids.append(id);
            }
        }
    }
    TRC_DEBUG("browser artwork: found {} Firefox session identities", ids.size());
#endif
    return ids;
}

QImage browserExecutableIcon(const QString& browser) {
    if(browser != QStringLiteral("chrome") && browser != QStringLiteral("msedge") &&
       browser != QStringLiteral("firefox") && browser != QStringLiteral("brave"))
        return {};
    const QString executable = browser + QStringLiteral(".exe");
    QString path = QStandardPaths::findExecutable(executable);
#ifdef Q_OS_WIN
    if(path.isEmpty()) {
        const QString key = QStringLiteral("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%1")
                                .arg(executable);
        for(const QString& root : {QStringLiteral("HKEY_CURRENT_USER\\"), QStringLiteral("HKEY_LOCAL_MACHINE\\")}) {
            QSettings registry(root + key, QSettings::NativeFormat);
            const QString candidate = registry.value(QStringLiteral("Default")).toString();
            if(QFileInfo(candidate).isFile()) {
                path = candidate;
                break;
            }
        }
    }
#endif
    if(path.isEmpty()) {
        TRC_DEBUG("browser artwork: executable not found for {}", browser.toStdString());
        return {};
    }
#ifdef Q_OS_WIN
    // The shell icon lookup needs COM. Non-Windows Qt platforms (offscreen) do not start it and the lookup crashes.
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#endif
    const QImage icon = QFileIconProvider().icon(QFileInfo(path)).pixmap(32, 32).toImage();
#ifdef Q_OS_WIN
    if(SUCCEEDED(comResult))
        CoUninitialize();
#endif
    if(icon.isNull())
        TRC_DEBUG("browser artwork: executable icon unavailable for {}", browser.toStdString());
    return icon;
}

double browserIconDifference(const QImage& image, const QImage& icon) {
    if(image.isNull() || icon.isNull())
        return 1.0;
    const double imageRatio = static_cast<double>(image.width()) / image.height();
    const double iconRatio = static_cast<double>(icon.width()) / icon.height();
    if(qAbs(imageRatio - iconRatio) > 0.1)
        return 1.0;
    const QImage scaledImage = image.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                   .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const QImage scaledIcon = icon.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                                  .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    qint64 difference = 0;
    for(int row = 0; row < 32; ++row) {
        const QRgb* imagePixels = reinterpret_cast<const QRgb*>(scaledImage.constScanLine(row));
        const QRgb* iconPixels = reinterpret_cast<const QRgb*>(scaledIcon.constScanLine(row));
        for(int column = 0; column < 32; ++column) {
            const QRgb imagePixel = imagePixels[column];
            const QRgb iconPixel = iconPixels[column];
            difference += qAbs(qAlpha(imagePixel) - qAlpha(iconPixel));
            difference += qAbs(qRed(imagePixel) - qRed(iconPixel));
            difference += qAbs(qGreen(imagePixel) - qGreen(iconPixel));
            difference += qAbs(qBlue(imagePixel) - qBlue(iconPixel));
        }
    }
    return static_cast<double>(difference) / (32 * 32 * 4 * 255);
}

QImage chooseBrowserArtwork(const QImage& mediaArt, const QImage& favicon, const QImage& browserIcon) {
    if(favicon.isNull())
        return mediaArt;
    if(mediaArt.isNull())
        return favicon;
    const double difference = browserIconDifference(mediaArt, browserIcon);
    TRC_DEBUG("browser artwork: thumbnail and executable icon difference {:.3f}", difference);
    return difference <= 0.15 ? favicon : mediaArt;
}

}