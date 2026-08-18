#include "Settings.h"

Settings::Settings(QObject* parent)
    : QObject(parent), m_store(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                               QStringLiteral("MediaFlyouts")) {
    reload();
}

void Settings::reload() {
    m_mediaFlyoutEnabled = m_store.value(QStringLiteral("mediaFlyoutEnabled"), true).toBool();
    m_seekbarEnabled = m_store.value(QStringLiteral("seekbarEnabled"), true).toBool();
    m_seekbarInteractive = m_store.value(QStringLiteral("seekbarInteractive"), true).toBool();
    m_mediaFlyoutAlwaysDisplay = m_store.value(QStringLiteral("mediaFlyoutAlwaysDisplay"), false).toBool();
    m_centerTitleArtist = m_store.value(QStringLiteral("centerTitleArtist"), false).toBool();
    m_compactLayout = m_store.value(QStringLiteral("compactLayout"), false).toBool();
    m_playerInfoEnabled = m_store.value(QStringLiteral("playerInfoEnabled"), true).toBool();
    m_repeatEnabled = m_store.value(QStringLiteral("repeatEnabled"), false).toBool();
    m_shuffleEnabled = m_store.value(QStringLiteral("shuffleEnabled"), false).toBool();
    m_volumeFlyoutEnabled = m_store.value(QStringLiteral("volumeFlyoutEnabled"), true).toBool();
    m_overrideNativeOsd = m_store.value(QStringLiteral("overrideNativeOsd"), true).toBool();
    m_lockKeysEnabled = m_store.value(QStringLiteral("lockKeysEnabled"), true).toBool();
    m_lockKeysCapsEnabled = m_store.value(QStringLiteral("lockKeysCapsEnabled"), true).toBool();
    m_lockKeysNumEnabled = m_store.value(QStringLiteral("lockKeysNumEnabled"), true).toBool();
    m_lockKeysScrollEnabled = m_store.value(QStringLiteral("lockKeysScrollEnabled"), true).toBool();
    m_lockKeysInsertEnabled = m_store.value(QStringLiteral("lockKeysInsertEnabled"), false).toBool();
    m_lockKeysBoldUi = m_store.value(QStringLiteral("lockKeysBoldUi"), true).toBool();
    m_lockKeysDuration = m_store.value(QStringLiteral("lockKeysDuration"), 2000).toInt();
    m_nextUpEnabled = m_store.value(QStringLiteral("nextUpEnabled"), true).toBool();
    m_disableOnFullscreen = m_store.value(QStringLiteral("disableOnFullscreen"), true).toBool();
    m_launchOnStartup = m_store.value(QStringLiteral("launchOnStartup"), false).toBool();
    m_trayIconHidden = m_store.value(QStringLiteral("trayIconHidden"), false).toBool();
    m_trayLeftClickShowsFlyout = m_store.value(QStringLiteral("trayLeftClickShowsFlyout"), false).toBool();
    m_taskbarWidgetEnabled = m_store.value(QStringLiteral("taskbarWidgetEnabled"), false).toBool();
    m_taskbarWidgetControlsEnabled = m_store.value(QStringLiteral("taskbarWidgetControlsEnabled"), true).toBool();
    // flyoutPosition indexes mf::FlyoutPosition (4 = BottomCenter); duration in milliseconds
    m_flyoutPosition = m_store.value(QStringLiteral("flyoutPosition"), 4).toInt();
    m_flyoutMonitor = m_store.value(QStringLiteral("flyoutMonitor"), 0).toInt();
    m_flyoutDuration = m_store.value(QStringLiteral("flyoutDuration"), 4500).toInt();
    m_flyoutAnimationSpeed = m_store.value(QStringLiteral("flyoutAnimationSpeed"), 2).toInt();
    m_flyoutAnimationEasingStyle = m_store.value(QStringLiteral("flyoutAnimationEasingStyle"), 3).toInt();
    m_volumeFlyoutDuration = m_store.value(QStringLiteral("volumeFlyoutDuration"), 1800).toInt();
    m_nextUpDuration = m_store.value(QStringLiteral("nextUpDuration"), 2000).toInt();
    m_taskbarWidgetOffset = m_store.value(QStringLiteral("taskbarWidgetOffset"), 0).toInt();
    // widget side: 0 = left, 1 = center, 2 = right
    m_taskbarWidgetSide = m_store.value(QStringLiteral("taskbarWidgetSide"), 2).toInt();
    m_taskbarWidgetMonitor = m_store.value(QStringLiteral("taskbarWidgetMonitor"), 0).toInt();
    m_taskbarWidgetShowPauseOverlay = m_store.value(QStringLiteral("taskbarWidgetShowPauseOverlay"), true).toBool();
    m_taskbarWidgetScrollingEnabled = m_store.value(QStringLiteral("taskbarWidgetScrollingEnabled"), false).toBool();
    m_taskbarVisualizerEnabled = m_store.value(QStringLiteral("taskbarVisualizerEnabled"), false).toBool();
    m_onboardingCompleted = m_store.value(QStringLiteral("onboardingCompleted"), false).toBool();
    m_accentFromArt = m_store.value(QStringLiteral("accentFromArt"), false).toBool();
    m_acrylicEnabled = m_store.value(QStringLiteral("acrylicEnabled"), false).toBool();
    // appTheme: 0 = follow system, 1 = light, 2 = dark
    m_appTheme = m_store.value(QStringLiteral("appTheme"), 0).toInt();
    // filterMode: 0 = off, 1 = blacklist (hide listed), 2 = whitelist (show only listed)
    m_filterMode = m_store.value(QStringLiteral("filterMode"), 0).toInt();
    m_filterList = m_store.value(QStringLiteral("filterList")).toStringList();
}

bool Settings::exportTo(const QUrl& file) const {
    const QString path = file.isLocalFile() ? file.toLocalFile() : file.toString();
    QSettings out(path, QSettings::IniFormat);
    if(out.status() != QSettings::NoError)
        return false;
    const QStringList keys = m_store.allKeys();
    for(const QString& key : keys)
        out.setValue(key, m_store.value(key));
    out.sync();
    return out.status() == QSettings::NoError;
}

bool Settings::importFrom(const QUrl& file) {
    const QString path = file.isLocalFile() ? file.toLocalFile() : file.toString();
    QSettings in(path, QSettings::IniFormat);
    if(in.status() != QSettings::NoError)
        return false;
    const QStringList keys = in.allKeys();
    for(const QString& key : keys)
        m_store.setValue(key, in.value(key));
    m_store.sync();
    reload();
    emit changed();
    return true;
}

void Settings::assignInt(const char* key, int& field, int v) {
    if(field == v)
        return;
    field = v;
    m_store.setValue(QString::fromLatin1(key), v);
    emit changed();
}

void Settings::assign(const char* key, bool& field, bool v) {
    if(field == v)
        return;
    field = v;
    m_store.setValue(QString::fromLatin1(key), v);
    emit changed();
}

bool Settings::mediaFlyoutEnabled() const {
    return m_mediaFlyoutEnabled;
}
bool Settings::seekbarEnabled() const {
    return m_seekbarEnabled;
}
bool Settings::seekbarInteractive() const {
    return m_seekbarInteractive;
}
bool Settings::mediaFlyoutAlwaysDisplay() const {
    return m_mediaFlyoutAlwaysDisplay;
}
bool Settings::centerTitleArtist() const {
    return m_centerTitleArtist;
}
bool Settings::compactLayout() const {
    return m_compactLayout;
}
bool Settings::playerInfoEnabled() const {
    return m_playerInfoEnabled;
}
bool Settings::repeatEnabled() const {
    return m_repeatEnabled;
}
bool Settings::shuffleEnabled() const {
    return m_shuffleEnabled;
}
bool Settings::volumeFlyoutEnabled() const {
    return m_volumeFlyoutEnabled;
}
bool Settings::overrideNativeOsd() const {
    return m_overrideNativeOsd;
}
bool Settings::lockKeysEnabled() const {
    return m_lockKeysEnabled;
}
bool Settings::lockKeysCapsEnabled() const {
    return m_lockKeysCapsEnabled;
}
bool Settings::lockKeysNumEnabled() const {
    return m_lockKeysNumEnabled;
}
bool Settings::lockKeysScrollEnabled() const {
    return m_lockKeysScrollEnabled;
}
bool Settings::lockKeysInsertEnabled() const {
    return m_lockKeysInsertEnabled;
}
bool Settings::lockKeysBoldUi() const {
    return m_lockKeysBoldUi;
}
int Settings::lockKeysDuration() const {
    return m_lockKeysDuration;
}
bool Settings::nextUpEnabled() const {
    return m_nextUpEnabled;
}
bool Settings::disableOnFullscreen() const {
    return m_disableOnFullscreen;
}
bool Settings::launchOnStartup() const {
    return m_launchOnStartup;
}
bool Settings::trayIconHidden() const {
    return m_trayIconHidden;
}
bool Settings::trayLeftClickShowsFlyout() const {
    return m_trayLeftClickShowsFlyout;
}
bool Settings::taskbarWidgetEnabled() const {
    return m_taskbarWidgetEnabled;
}
bool Settings::taskbarWidgetControlsEnabled() const {
    return m_taskbarWidgetControlsEnabled;
}
int Settings::flyoutPosition() const {
    return m_flyoutPosition;
}
int Settings::flyoutMonitor() const {
    return m_flyoutMonitor;
}
int Settings::flyoutDuration() const {
    return m_flyoutDuration;
}
int Settings::flyoutAnimationSpeed() const {
    return m_flyoutAnimationSpeed;
}
int Settings::flyoutAnimationEasingStyle() const {
    return m_flyoutAnimationEasingStyle;
}
int Settings::flyoutAnimationMs() const {
    switch(m_flyoutAnimationSpeed) {
    case 0:
        return 0; // off
    case 1:
        return 150; // 0.5x
    case 2:
        return 300; // 1x
    case 3:
        return 450; // 1.5x
    case 4:
        return 600; // 2x
    default:
        return 900; // 3x
    }
}
int Settings::volumeFlyoutDuration() const {
    return m_volumeFlyoutDuration;
}
int Settings::nextUpDuration() const {
    return m_nextUpDuration;
}
int Settings::taskbarWidgetOffset() const {
    return m_taskbarWidgetOffset;
}
int Settings::appTheme() const {
    return m_appTheme;
}
int Settings::filterMode() const {
    return m_filterMode;
}
QStringList Settings::filterList() const {
    return m_filterList;
}
bool Settings::playerAllowed(const QString& player) const {
    if(m_filterMode == 0)
        return true;
    bool listed = false;
    for(const QString& e : m_filterList) {
        if(!player.isEmpty() &&
           (player.compare(e, Qt::CaseInsensitive) == 0 || player.contains(e, Qt::CaseInsensitive))) {
            listed = true;
            break;
        }
    }
    // blacklist hides listed players; whitelist shows only listed players
    return m_filterMode == 1 ? !listed : listed;
}
int Settings::taskbarWidgetSide() const {
    return m_taskbarWidgetSide;
}
int Settings::taskbarWidgetMonitor() const {
    return m_taskbarWidgetMonitor;
}
bool Settings::taskbarWidgetShowPauseOverlay() const {
    return m_taskbarWidgetShowPauseOverlay;
}
bool Settings::taskbarWidgetScrollingEnabled() const {
    return m_taskbarWidgetScrollingEnabled;
}
bool Settings::taskbarVisualizerEnabled() const {
    return m_taskbarVisualizerEnabled;
}
bool Settings::onboardingCompleted() const {
    return m_onboardingCompleted;
}
bool Settings::accentFromArt() const {
    return m_accentFromArt;
}
bool Settings::acrylicEnabled() const {
    return m_acrylicEnabled;
}

void Settings::setMediaFlyoutEnabled(bool v) {
    assign("mediaFlyoutEnabled", m_mediaFlyoutEnabled, v);
}
void Settings::setSeekbarEnabled(bool v) {
    assign("seekbarEnabled", m_seekbarEnabled, v);
}
void Settings::setSeekbarInteractive(bool v) {
    assign("seekbarInteractive", m_seekbarInteractive, v);
}
void Settings::setMediaFlyoutAlwaysDisplay(bool v) {
    assign("mediaFlyoutAlwaysDisplay", m_mediaFlyoutAlwaysDisplay, v);
}
void Settings::setCenterTitleArtist(bool v) {
    assign("centerTitleArtist", m_centerTitleArtist, v);
}
void Settings::setCompactLayout(bool v) {
    assign("compactLayout", m_compactLayout, v);
}
void Settings::setPlayerInfoEnabled(bool v) {
    assign("playerInfoEnabled", m_playerInfoEnabled, v);
}
void Settings::setRepeatEnabled(bool v) {
    assign("repeatEnabled", m_repeatEnabled, v);
}
void Settings::setShuffleEnabled(bool v) {
    assign("shuffleEnabled", m_shuffleEnabled, v);
}
void Settings::setVolumeFlyoutEnabled(bool v) {
    assign("volumeFlyoutEnabled", m_volumeFlyoutEnabled, v);
}
void Settings::setOverrideNativeOsd(bool v) {
    assign("overrideNativeOsd", m_overrideNativeOsd, v);
}
void Settings::setLockKeysEnabled(bool v) {
    assign("lockKeysEnabled", m_lockKeysEnabled, v);
}
void Settings::setLockKeysCapsEnabled(bool v) {
    assign("lockKeysCapsEnabled", m_lockKeysCapsEnabled, v);
}
void Settings::setLockKeysNumEnabled(bool v) {
    assign("lockKeysNumEnabled", m_lockKeysNumEnabled, v);
}
void Settings::setLockKeysScrollEnabled(bool v) {
    assign("lockKeysScrollEnabled", m_lockKeysScrollEnabled, v);
}
void Settings::setLockKeysInsertEnabled(bool v) {
    assign("lockKeysInsertEnabled", m_lockKeysInsertEnabled, v);
}
void Settings::setLockKeysBoldUi(bool v) {
    assign("lockKeysBoldUi", m_lockKeysBoldUi, v);
}
void Settings::setLockKeysDuration(int v) {
    assignInt("lockKeysDuration", m_lockKeysDuration, v);
}
void Settings::setNextUpEnabled(bool v) {
    assign("nextUpEnabled", m_nextUpEnabled, v);
}
void Settings::setDisableOnFullscreen(bool v) {
    assign("disableOnFullscreen", m_disableOnFullscreen, v);
}
void Settings::setLaunchOnStartup(bool v) {
    assign("launchOnStartup", m_launchOnStartup, v);
}
void Settings::setTrayIconHidden(bool v) {
    assign("trayIconHidden", m_trayIconHidden, v);
}
void Settings::setTrayLeftClickShowsFlyout(bool v) {
    assign("trayLeftClickShowsFlyout", m_trayLeftClickShowsFlyout, v);
}
void Settings::setTaskbarWidgetEnabled(bool v) {
    assign("taskbarWidgetEnabled", m_taskbarWidgetEnabled, v);
}
void Settings::setTaskbarWidgetControlsEnabled(bool v) {
    assign("taskbarWidgetControlsEnabled", m_taskbarWidgetControlsEnabled, v);
}
void Settings::setFlyoutPosition(int v) {
    assignInt("flyoutPosition", m_flyoutPosition, v);
}
void Settings::setFlyoutMonitor(int v) {
    assignInt("flyoutMonitor", m_flyoutMonitor, v);
}
void Settings::setFlyoutDuration(int v) {
    assignInt("flyoutDuration", m_flyoutDuration, v);
}
void Settings::setFlyoutAnimationSpeed(int v) {
    assignInt("flyoutAnimationSpeed", m_flyoutAnimationSpeed, v);
}
void Settings::setFlyoutAnimationEasingStyle(int v) {
    assignInt("flyoutAnimationEasingStyle", m_flyoutAnimationEasingStyle, v);
}
void Settings::setVolumeFlyoutDuration(int v) {
    assignInt("volumeFlyoutDuration", m_volumeFlyoutDuration, v);
}
void Settings::setNextUpDuration(int v) {
    assignInt("nextUpDuration", m_nextUpDuration, v);
}
void Settings::setTaskbarWidgetOffset(int v) {
    assignInt("taskbarWidgetOffset", m_taskbarWidgetOffset, v);
}
void Settings::setAppTheme(int v) {
    assignInt("appTheme", m_appTheme, v);
}
void Settings::setFilterMode(int v) {
    assignInt("filterMode", m_filterMode, v);
}
void Settings::addFilterEntry(const QString& name) {
    const QString trimmed = name.trimmed();
    if(trimmed.isEmpty())
        return;
    for(const QString& e : m_filterList)
        if(e.compare(trimmed, Qt::CaseInsensitive) == 0)
            return;
    m_filterList.append(trimmed);
    m_store.setValue(QStringLiteral("filterList"), m_filterList);
    emit changed();
}
void Settings::removeFilterEntry(const QString& name) {
    bool removed = false;
    for(int i = m_filterList.size() - 1; i >= 0; --i)
        if(m_filterList.at(i).compare(name, Qt::CaseInsensitive) == 0) {
            m_filterList.removeAt(i);
            removed = true;
        }
    if(!removed)
        return;
    m_store.setValue(QStringLiteral("filterList"), m_filterList);
    emit changed();
}
void Settings::setTaskbarWidgetSide(int v) {
    assignInt("taskbarWidgetSide", m_taskbarWidgetSide, v);
}
void Settings::setTaskbarWidgetMonitor(int v) {
    assignInt("taskbarWidgetMonitor", m_taskbarWidgetMonitor, v);
}
void Settings::setTaskbarWidgetShowPauseOverlay(bool v) {
    assign("taskbarWidgetShowPauseOverlay", m_taskbarWidgetShowPauseOverlay, v);
}
void Settings::setTaskbarWidgetScrollingEnabled(bool v) {
    assign("taskbarWidgetScrollingEnabled", m_taskbarWidgetScrollingEnabled, v);
}
void Settings::setTaskbarVisualizerEnabled(bool v) {
    assign("taskbarVisualizerEnabled", m_taskbarVisualizerEnabled, v);
}
void Settings::setOnboardingCompleted(bool v) {
    assign("onboardingCompleted", m_onboardingCompleted, v);
}
void Settings::setAccentFromArt(bool v) {
    assign("accentFromArt", m_accentFromArt, v);
}
void Settings::setAcrylicEnabled(bool v) {
    assign("acrylicEnabled", m_acrylicEnabled, v);
}
