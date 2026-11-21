#include "UserGuide.h"

#include <QFile>

#include "Logging.h"

namespace mf {

QString userGuideText() {
    QFile file(QStringLiteral(":/docs/user/user-guide.md"));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        TRC_WARN("user guide: resource missing ({})", file.errorString().toStdString());
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace mf
