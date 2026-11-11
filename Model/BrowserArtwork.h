#pragma once

#include <QString>
#include <QStringList>

#include "TrackInfo.h"

namespace mf {

bool matchesBrowserArtwork(const TrackInfo& track, const QString& browser, const QString& tabTitle,
                           int audibleTabCount, const QStringList& firefoxAppIds = {});

}