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

bool sameDpi(int a, int b) {
    // Unknown on either side: nothing to compare.
    return a <= 0 || b <= 0 || a == b;
}

} // namespace

QSize nativeSizeOf(const QSize& logicalSize, qreal devicePixelRatio) {
    const qreal dpr = safeDpr(devicePixelRatio);
    return QSize(qRound(logicalSize.width() * dpr), qRound(logicalSize.height() * dpr));
}

bool scaleIsSettled(int nativeDpi, int screenDpi) {
    return sameDpi(nativeDpi, screenDpi);
}

bool scaleIsSettled(const WindowScaleState& state) {
    return scaleIsSettled(state.nativeDpi, state.screenDpi);
}

bool resizeIsAuthoritative(const WindowScaleSnapshot& snapshot, const WindowScaleState& state) {
    if(!scaleIsSettled(state))
        return false;
    if(!qFuzzyCompare(safeDpr(snapshot.devicePixelRatio), safeDpr(state.devicePixelRatio)))
        return false;
    if(!sameDpi(snapshot.nativeDpi, state.nativeDpi))
        return false;
    // A stale event reports a size Qt no longer agrees with.
    return !state.reportedSize.isValid() || !state.currentSize.isValid()
           || nearlyEqual(state.reportedSize, state.currentSize);
}

QSize sizeToRestore(const WindowScaleSnapshot& snapshot, const WindowScaleState& state) {
    if(!snapshot.logicalSize.isValid() || !scaleIsSettled(state))
        return QSize();
    const bool currentOk = !state.currentSize.isValid() || nearlyEqual(state.currentSize, snapshot.logicalSize);
    const bool reportedOk = !state.reportedSize.isValid() || nearlyEqual(state.reportedSize, snapshot.logicalSize);
    if(currentOk && reportedOk)
        return QSize();
    return snapshot.logicalSize;
}

bool needsNudge(const QSize& target, const WindowScaleState& state) {
    if(!target.isValid() || !state.currentSize.isValid() || !state.reportedSize.isValid())
        return false;
    return state.currentSize == target && state.reportedSize != target;
}

} // namespace mf
