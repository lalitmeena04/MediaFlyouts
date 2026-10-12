#pragma once

#include <QSize>
#include <QtGlobal>

namespace mf {

// Last size a window chose on its own, and the scale it chose it at.
struct WindowScaleSnapshot {
        QSize logicalSize;
        qreal devicePixelRatio = 1.0;
};

// Native size of a logical size at a scale. Rounds like Qt does.
QSize nativeSizeOf(const QSize& logicalSize, qreal devicePixelRatio);

// True when a resize came from the app at the snapshot scale. Record it.
// False while the scale is changing. Qt then reports the old native size
// divided by the new scale, which is not a size the app asked for.
bool resizeIsAuthoritative(const WindowScaleSnapshot& snapshot, qreal currentDpr);

// Size to re-apply after a scale change. Invalid when nothing is wrong.
// Fires only when the native size did not follow the scale. That is the
// shrink that happens when Qt skips the WM_DPICHANGED resize.
QSize sizeToRestore(const WindowScaleSnapshot& snapshot, const QSize& currentNativeSize, qreal currentDpr);

} // namespace mf
