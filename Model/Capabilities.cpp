#include "Capabilities.h"

#include "Logging.h"

Capabilities::Capabilities(QObject* parent) : QObject(parent) {}

bool Capabilities::audioAvailable() const {
    return m_audioAvailable;
}

bool Capabilities::mediaSessionAvailable() const {
    return m_mediaSessionAvailable;
}

bool Capabilities::lockKeysAvailable() const {
    return m_lockKeysAvailable;
}

bool Capabilities::mediaKeysAvailable() const {
    return m_mediaKeysAvailable;
}

bool Capabilities::chargerAvailable() const {
    return m_chargerAvailable;
}

// A missing charger is normal on a desktop, so it never counts as degraded.
QStringList Capabilities::unavailableNames() const {
    QStringList names;
    if(!m_audioAvailable)
        names << tr("Audio devices");
    if(!m_mediaSessionAvailable)
        names << tr("Media sessions");
    if(!m_lockKeysAvailable)
        names << tr("Lock keys");
    if(!m_mediaKeysAvailable)
        names << tr("Media keys");
    return names;
}

bool Capabilities::degraded() const {
    return !unavailableNames().isEmpty();
}

QString Capabilities::statusSummary() const {
    const QStringList names = unavailableNames();
    if(names.isEmpty())
        return QStringLiteral("MediaFlyouts");
    return tr("MediaFlyouts\nUnavailable: %1").arg(names.join(QStringLiteral(", ")));
}

void Capabilities::update(bool& field, bool available, const char* name) {
    if(field == available)
        return;
    field = available;
    TRC_INFO("capability {} -> {}", name, available);
    emit changed();
}

void Capabilities::setAudioAvailable(bool available) {
    update(m_audioAvailable, available, "audioAvailable");
}

void Capabilities::setMediaSessionAvailable(bool available) {
    update(m_mediaSessionAvailable, available, "mediaSessionAvailable");
}

void Capabilities::setLockKeysAvailable(bool available) {
    update(m_lockKeysAvailable, available, "lockKeysAvailable");
}

void Capabilities::setMediaKeysAvailable(bool available) {
    update(m_mediaKeysAvailable, available, "mediaKeysAvailable");
}

void Capabilities::setChargerAvailable(bool available) {
    update(m_chargerAvailable, available, "chargerAvailable");
}
