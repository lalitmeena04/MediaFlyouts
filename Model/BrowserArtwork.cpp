#include "BrowserArtwork.h"

namespace mf {

namespace {
bool isChromiumWebAppId(const QString& source, const QString& browser) {
    const QString prefix = browser + QStringLiteral("._crx_");
    if(source.size() != prefix.size() + 32 || !source.startsWith(prefix))
        return false;
    for(int i = prefix.size(); i < source.size(); ++i) {
        const char16_t c = source.at(i).unicode();
        if(c < u'a' || c > u'p')
            return false;
    }
    return true;
}
} // namespace

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
    const bool webApp = !executable && !registeredFirefox && browser != QStringLiteral("firefox") &&
                        isChromiumWebAppId(source, browser);
    if(!executable && !webApp && !registeredFirefox)
        return false;

    const QString title = track.title.simplified();
    return !title.isEmpty() && tabTitle.simplified().contains(title, Qt::CaseInsensitive);
}

}