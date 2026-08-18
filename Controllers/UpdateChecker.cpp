#include "UpdateChecker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {
// GitHub releases endpoint for the project; a 404 simply means "couldn't check".
constexpr char kReleasesApi[] = "https://api.github.com/repos/mediaflyouts/mediaflyouts/releases/latest";

// compare dotted numeric versions; returns true if b is strictly newer than a
bool isNewer(const QString& a, const QString& b) {
    const QStringList pa = QString(a).remove('v').split('.');
    const QStringList pb = QString(b).remove('v').split('.');
    const int n = qMax(pa.size(), pb.size());
    for(int i = 0; i < n; ++i) {
        const int va = i < pa.size() ? pa[i].toInt() : 0;
        const int vb = i < pb.size() ? pb[i].toInt() : 0;
        if(vb != va)
            return vb > va;
    }
    return false;
}
} // namespace

UpdateChecker::UpdateChecker(QObject* parent) : QObject(parent), m_net(new QNetworkAccessManager(this)) {}

QString UpdateChecker::status() const {
    return m_status;
}
QString UpdateChecker::currentVersion() const {
    return m_current;
}
QString UpdateChecker::latestVersion() const {
    return m_latest;
}
bool UpdateChecker::updateAvailable() const {
    return m_available;
}
bool UpdateChecker::checking() const {
    return m_checking;
}

void UpdateChecker::check() {
    if(m_checking)
        return;
    m_checking = true;
    m_status = QStringLiteral("Checking for updates…");
    emit changed();

    QNetworkRequest request{QUrl(QString::fromLatin1(kReleasesApi))};
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("MediaFlyouts"));
    QNetworkReply* reply = m_net->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        m_checking = false;

        if(reply->error() != QNetworkReply::NoError) {
            m_status = QStringLiteral("Couldn't check for updates right now.");
            emit changed();
            return;
        }

        const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        const QString tag = obj.value(QStringLiteral("tag_name")).toString();
        if(tag.isEmpty()) {
            m_status = QStringLiteral("No releases found.");
            emit changed();
            return;
        }

        m_latest = tag;
        m_available = isNewer(m_current, tag);
        m_status = m_available ? QStringLiteral("Update available: %1").arg(tag)
                               : QStringLiteral("You're on the latest version.");
        emit changed();
    });
}
