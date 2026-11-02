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

void Capabilities::setAudioAvailable(bool available) {
    if(m_audioAvailable == available)
        return;
    m_audioAvailable = available;
    TRC_INFO("capability audioAvailable -> {}", available);
    emit changed();
}

void Capabilities::setMediaSessionAvailable(bool available) {
    if(m_mediaSessionAvailable == available)
        return;
    m_mediaSessionAvailable = available;
    TRC_INFO("capability mediaSessionAvailable -> {}", available);
    emit changed();
}

void Capabilities::setLockKeysAvailable(bool available) {
    if(m_lockKeysAvailable == available)
        return;
    m_lockKeysAvailable = available;
    TRC_INFO("capability lockKeysAvailable -> {}", available);
    emit changed();
}

void Capabilities::setMediaKeysAvailable(bool available) {
    if(m_mediaKeysAvailable == available)
        return;
    m_mediaKeysAvailable = available;
    TRC_INFO("capability mediaKeysAvailable -> {}", available);
    emit changed();
}

void Capabilities::setChargerAvailable(bool available) {
    if(m_chargerAvailable == available)
        return;
    m_chargerAvailable = available;
    TRC_INFO("capability chargerAvailable -> {}", available);
    emit changed();
}
