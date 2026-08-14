#include "Settings.h"

Settings::Settings(QObject* parent)
    : QObject(parent), m_store(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                               QStringLiteral("MediaFlyouts")) {
    reload();
}

void Settings::reload() {
    m_mediaFlyoutEnabled = m_store.value(QStringLiteral("mediaFlyoutEnabled"), true).toBool();
    m_seekbarEnabled = m_store.value(QStringLiteral("seekbarEnabled"), true).toBool();
    m_mediaFlyoutAlwaysDisplay = m_store.value(QStringLiteral("mediaFlyoutAlwaysDisplay"), false).toBool();
    m_centerTitleArtist = m_store.value(QStringLiteral("centerTitleArtist"), false).toBool();
    m_compactLayout = m_store.value(QStringLiteral("compactLayout"), false).toBool();
    m_volumeFlyoutEnabled = m_store.value(QStringLiteral("volumeFlyoutEnabled"), true).toBool();
    m_overrideNativeOsd = m_store.value(QStringLiteral("overrideNativeOsd"), true).toBool();
    m_lockKeysEnabled = m_store.value(QStringLiteral("lockKeysEnabled"), true).toBool();
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
    m_volumeFlyoutDuration = m_store.value(QStringLiteral("volumeFlyoutDuration"), 1800).toInt();
    m_nextUpDuration = m_store.value(QStringLiteral("nextUpDuration"), 2000).toInt();
    m_taskbarWidgetOffset = m_store.value(QStringLiteral("taskbarWidgetOffset"), 0).toInt();
    m_taskbarWidgetSide = m_store.value(QStringLiteral("taskbarWidgetSide"), 1).toInt();
    m_taskbarWidgetMonitor = m_store.value(QStringLiteral("taskbarWidgetMonitor"), 0).toInt();
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
bool Settings::mediaFlyoutAlwaysDisplay() const {
    return m_mediaFlyoutAlwaysDisplay;
}
bool Settings::centerTitleArtist() const {
    return m_centerTitleArtist;
}
bool Settings::compactLayout() const {
    return m_compactLayout;
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
int Settings::taskbarWidgetSide() const {
    return m_taskbarWidgetSide;
}
int Settings::taskbarWidgetMonitor() const {
    return m_taskbarWidgetMonitor;
}

void Settings::setMediaFlyoutEnabled(bool v) {
    assign("mediaFlyoutEnabled", m_mediaFlyoutEnabled, v);
}
void Settings::setSeekbarEnabled(bool v) {
    assign("seekbarEnabled", m_seekbarEnabled, v);
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
void Settings::setVolumeFlyoutEnabled(bool v) {
    assign("volumeFlyoutEnabled", m_volumeFlyoutEnabled, v);
}
void Settings::setOverrideNativeOsd(bool v) {
    assign("overrideNativeOsd", m_overrideNativeOsd, v);
}
void Settings::setLockKeysEnabled(bool v) {
    assign("lockKeysEnabled", m_lockKeysEnabled, v);
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
void Settings::setVolumeFlyoutDuration(int v) {
    assignInt("volumeFlyoutDuration", m_volumeFlyoutDuration, v);
}
void Settings::setNextUpDuration(int v) {
    assignInt("nextUpDuration", m_nextUpDuration, v);
}
void Settings::setTaskbarWidgetOffset(int v) {
    assignInt("taskbarWidgetOffset", m_taskbarWidgetOffset, v);
}
void Settings::setTaskbarWidgetSide(int v) {
    assignInt("taskbarWidgetSide", m_taskbarWidgetSide, v);
}
void Settings::setTaskbarWidgetMonitor(int v) {
    assignInt("taskbarWidgetMonitor", m_taskbarWidgetMonitor, v);
}
