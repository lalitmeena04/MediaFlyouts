#include "WhatsNew.h"

#include <QFile>
#include <QRegularExpression>

#include "Logging.h"

namespace mf {

QString baseVersion(const QString& version) {
    QString v = version.trimmed();
    if(v.startsWith(QLatin1Char('v'), Qt::CaseInsensitive))
        v.remove(0, 1);
    return v.section(QLatin1Char('-'), 0, 0);
}

bool shouldShowWhatsNew(const QString& lastSeenVersion, const QString& currentVersion) {
    const QString last = baseVersion(lastSeenVersion);
    const QString current = baseVersion(currentVersion);
    return !last.isEmpty() && !current.isEmpty() && last != current;
}

QString whatsNewText() {
    QFile file(QStringLiteral(":/whatsnew/WHATS_NEW.md"));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        TRC_WARN("whats new: resource missing");
        return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
}

QString whatsNewVersion(const QString& text) {
    static const QRegularExpression re(QStringLiteral("^\\s*Version\\s+(\\d+\\.\\d+\\.\\d+)\\s*$"),
                                       QRegularExpression::MultilineOption);
    const QString first = text.section(QLatin1Char('\n'), 0, 0);
    const QRegularExpressionMatch m = re.match(first);
    return m.hasMatch() ? m.captured(1) : QString();
}

QString whatsNewBody(const QString& text) {
    if(whatsNewVersion(text).isEmpty())
        return text.trimmed();
    return text.section(QLatin1Char('\n'), 1).trimmed();
}

} // namespace mf
