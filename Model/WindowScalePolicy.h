#pragma once

#include <QSize>
#include <QtGlobal>

namespace mf {

struct WindowScaleSnapshot {
        QSize logicalSize;
        qreal devicePixelRatio = 1.0;
};

QSize nativeSizeOf(const QSize& logicalSize, qreal devicePixelRatio);

// False while the scale is changing. Qt then reports the old native size
// divided by the new scale, which is not a size the app asked for.
bool resizeIsAuthoritative(const WindowScaleSnapshot& snapshot, qreal currentDpr);

// Fires only when the native size did not follow the scale. That is the
// shrink that happens when Qt skips the WM_DPICHANGED resize.
QSize sizeToRestore(const WindowScaleSnapshot& snapshot, const QSize& currentNativeSize, qreal currentDpr);

} // namespace mf
