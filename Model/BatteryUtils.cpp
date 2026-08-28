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

QString categoryFromClassOfDevice(quint32 classOfDevice) {
    const quint32 majorClass = (classOfDevice >> 8) & 0x1F;
    const quint32 minorClass = (classOfDevice >> 2) & 0x3F;

    if(majorClass == 0x04) // Audio/Video
        return QStringLiteral("headset");
    if(majorClass == 0x05) { // Peripheral (HID)
        const quint32 pointerKeyboardBits = (minorClass >> 4) & 0x3;
        if(pointerKeyboardBits == 0x1)
            return QStringLiteral("keyboard");
        if(pointerKeyboardBits == 0x2)
            return QStringLiteral("mouse");
        if(pointerKeyboardBits == 0x3)
            return QStringLiteral("keyboard"); // combo keyboard/pointing device

        const quint32 subtype = minorClass & 0xF;
        if(subtype == 0x1 || subtype == 0x2) // joystick / gamepad
            return QStringLiteral("gamepad");
    }
    return {};
}

} // namespace BatteryUtils
