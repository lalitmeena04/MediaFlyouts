#include "Settings.h"

Settings::Settings(QObject* parent)
    : QObject(parent), m_store(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("MediaFlyouts"),
                               QStringLiteral("MediaFlyouts")) {
    m_mediaFlyoutEnabled = m_store.value(QStringLiteral("mediaFlyoutEnabled"), true).toBool();
    m_volumeFlyoutEnabled = m_store.value(QStringLiteral("volumeFlyoutEnabled"), true).toBool();
    m_overrideNativeOsd = m_store.value(QStringLiteral("overrideNativeOsd"), true).toBool();
    m_lockKeysEnabled = m_store.value(QStringLiteral("lockKeysEnabled"), true).toBool();
    m_nextUpEnabled = m_store.value(QStringLiteral("nextUpEnabled"), true).toBool();
    m_disableOnFullscreen = m_store.value(QStringLiteral("disableOnFullscreen"), true).toBool();
    m_launchOnStartup = m_store.value(QStringLiteral("launchOnStartup"), false).toBool();
    m_taskbarWidgetEnabled = m_store.value(QStringLiteral("taskbarWidgetEnabled"), false).toBool();
    m_taskbarWidgetControlsEnabled = m_store.value(QStringLiteral("taskbarWidgetControlsEnabled"), true).toBool();
    // flyoutPosition indexes mf::FlyoutPosition (4 = BottomCenter); duration in milliseconds
    m_flyoutPosition = m_store.value(QStringLiteral("flyoutPosition"), 4).toInt();
    m_flyoutMonitor = m_store.value(QStringLiteral("flyoutMonitor"), 0).toInt();
    m_flyoutDuration = m_store.value(QStringLiteral("flyoutDuration"), 4500).toInt();
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

void Settings::setMediaFlyoutEnabled(bool v) {
    assign("mediaFlyoutEnabled", m_mediaFlyoutEnabled, v);
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
