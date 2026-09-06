#include "UpdateChecker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {
// GitHub releases endpoint for project. 404 means check failed.
constexpr char kReleasesApi[] = "https://api.github.com/repos/scleaverzer0ne/MediaFlyouts/releases/latest";
} // namespace

UpdateChecker::UpdateChecker(QObject* parent) : QObject(parent), m_net(new QNetworkAccessManager(this)) {}

bool UpdateChecker::isNewerVersion(const QString& current, const QString& latest) {
    // Compare numeric core only. Dev builds have build-number suffix.
    const QStringList pa = QString(current).remove('v').section('-', 0, 0).split('.');
    const QStringList pb = QString(latest).remove('v').section('-', 0, 0).split('.');
    const int n = qMax(pa.size(), pb.size());
    for(int i = 0; i < n; ++i) {
        const int va = i < pa.size() ? pa[i].toInt() : 0;
        const int vb = i < pb.size() ? pb[i].toInt() : 0;
        if(vb != va)
            return vb > va;
    }
    return false;
}

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
        m_available = isNewerVersion(m_current, tag);
        m_status = m_available ? QStringLiteral("Update available: %1").arg(tag)
                               : QStringLiteral("You're on the latest version.");
        emit changed();
    });
}
