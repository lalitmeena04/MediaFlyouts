#include "WhatsNewController.h"

#include "Logging.h"
#include "Settings.h"
#include "Version.h"
#include "WhatsNew.h"

WhatsNewController::WhatsNewController(Settings* settings, QObject* parent)
    : WhatsNewController(settings, QStringLiteral(MEDIAFLYOUTS_VERSION), parent) {}

WhatsNewController::WhatsNewController(Settings* settings, const QString& currentVersion, QObject* parent)
    : QObject(parent), m_settings(settings), m_current(mf::baseVersion(currentVersion)), m_text(mf::whatsNewText()) {}

void WhatsNewController::start() {
    const QString last = m_settings ? m_settings->lastSeenVersion() : QString();
    m_pending = mf::shouldShowWhatsNew(last, m_current);
    TRC_INFO("whats new: lastSeen='{}' current='{}' notesFor='{}' pending={}", last.toStdString(),
             m_current.toStdString(), mf::whatsNewVersion(m_text).toStdString(), m_pending);
    if(last.isEmpty())
        markSeen();
    emit changed();
}

bool WhatsNewController::pending() const {
    return m_pending;
}

QString WhatsNewController::version() const {
    return m_current;
}

QString WhatsNewController::text() const {
    return m_text;
}

QString WhatsNewController::body() const {
    return mf::whatsNewBody(m_text);
}

QString WhatsNewController::currentVersion() const {
    return m_current;
}

void WhatsNewController::markSeen() {
    if(m_settings)
        m_settings->setLastSeenVersion(m_current);
    if(!m_pending)
        return;
    m_pending = false;
    emit changed();
}

void WhatsNewController::requestShow() {
    emit showRequested();
}
