#include "BatteryUtils.h"

#include <QRegularExpression>

#include <algorithm>

namespace BatteryUtils {

QString cleanBluetoothProfileSuffix(const QString& rawName) {
    static const QRegularExpression suffixes(
        QStringLiteral(R"(\s+(Hands-Free AG|Hands-Free|Handsfree|Avrcp Transport)$)"),
        QRegularExpression::CaseInsensitiveOption);
    QString name = rawName;
    name.remove(suffixes);
    return name;
}

int milliwattHoursToPercent(long long remainingMwh, long long fullChargeMwh) {
    if(fullChargeMwh <= 0 || remainingMwh < 0)
        return -1;
    const long long percent = (100 * remainingMwh) / fullChargeMwh;
    return static_cast<int>(std::clamp<long long>(percent, 0, 100));
}

} // namespace BatteryUtils
