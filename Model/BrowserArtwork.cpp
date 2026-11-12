#include "BrowserArtwork.h"

#include <QRegularExpression>

namespace mf {

bool matchesBrowserArtwork(const TrackInfo& track, const QString& browser, const QString& tabTitle,
                           int audibleTabCount, const QStringList& firefoxAppIds) {
     if(track.state == PlaybackState::Stopped || audibleTabCount < 0 || audibleTabCount > 1 ||
         (track.state == PlaybackState::Paused && audibleTabCount != 0))
        return false;

    QString source = track.sourceApp.trimmed().toLower();
    source.replace(QLatin1Char('\\'), QLatin1Char('/'));
    source = source.mid(source.lastIndexOf(QLatin1Char('/')) + 1);
    if(browser != QStringLiteral("chrome") && browser != QStringLiteral("msedge") &&
         browser != QStringLiteral("firefox") && browser != QStringLiteral("brave"))
        return false;
    const bool executable = source == browser || source == browser + QStringLiteral(".exe");
    const bool registeredFirefox = browser == QStringLiteral("firefox") &&
                                   firefoxAppIds.contains(source, Qt::CaseInsensitive);
    const QRegularExpression webAppPattern(QStringLiteral("^%1\\._crx_[a-p]{32}$").arg(browser));
    const bool webApp = browser != QStringLiteral("firefox") && webAppPattern.match(source).hasMatch();
    if(!executable && !webApp && !registeredFirefox)
        return false;

    const QString title = track.title.simplified();
    return !title.isEmpty() && tabTitle.simplified().contains(title, Qt::CaseInsensitive);
}

}