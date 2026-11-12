#pragma once

#include <QImage>
#include <QString>
#include <QStringList>

namespace mf {

QImage browserExecutableIcon(const QString& browser);
QStringList firefoxAppUserModelIds();
double browserIconDifference(const QImage& image, const QImage& icon);
QImage chooseBrowserArtwork(const QImage& mediaArt, const QImage& favicon, const QImage& browserIcon);

}