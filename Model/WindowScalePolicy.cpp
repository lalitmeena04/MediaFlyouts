#include "WindowScalePolicy.h"

#include <QtMath>

namespace mf {

namespace {

qreal safeDpr(qreal dpr) {
    return dpr > 0 ? dpr : 1.0;
}

// Allow one pixel of rounding per axis.
bool nearlyEqual(const QSize& a, const QSize& b) {
    return qAbs(a.width() - b.width()) <= 1 && qAbs(a.height() - b.height()) <= 1;
}

} // namespace

QSize nativeSizeOf(const QSize& logicalSize, qreal devicePixelRatio) {
    const qreal dpr = safeDpr(devicePixelRatio);
    return QSize(qRound(logicalSize.width() * dpr), qRound(logicalSize.height() * dpr));
}

bool resizeIsAuthoritative(const WindowScaleSnapshot& snapshot, qreal currentDpr) {
    return qFuzzyCompare(safeDpr(snapshot.devicePixelRatio), safeDpr(currentDpr));
}

QSize sizeToRestore(const WindowScaleSnapshot& snapshot, const QSize& currentNativeSize, qreal currentDpr) {
    if(!snapshot.logicalSize.isValid() || !currentNativeSize.isValid())
        return QSize();
    const qreal oldDpr = safeDpr(snapshot.devicePixelRatio);
    const qreal newDpr = safeDpr(currentDpr);
    if(qFuzzyCompare(oldDpr, newDpr))
        return QSize();
    // Native followed the scale. Window is fine.
    if(nearlyEqual(currentNativeSize, nativeSizeOf(snapshot.logicalSize, newDpr)))
        return QSize();
    // Native stayed at the old scale. Re-apply the logical size.
    if(nearlyEqual(currentNativeSize, nativeSizeOf(snapshot.logicalSize, oldDpr)))
        return snapshot.logicalSize;
    return QSize();
}

} // namespace mf
