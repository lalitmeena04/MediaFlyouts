#pragma once

#include <QSize>
#include <QtGlobal>

namespace mf {

// The size the app asked for, and the scale it asked at.
struct WindowScaleSnapshot {
        QSize logicalSize;
        qreal devicePixelRatio = 1.0;
        // DPI of the native window at that time. 0 = unknown.
        int nativeDpi = 0;
};

// What the window reports right now.
struct WindowScaleState {
        // Size from the last resize event. Invalid = no event seen yet.
        QSize reportedSize;
        // Size Qt computes from the native rect now.
        QSize currentSize;
        qreal devicePixelRatio = 1.0;
        // DPI of the native window. 0 = unknown.
        int nativeDpi = 0;
        // DPI of the screen Qt binds the window to. 0 = unknown.
        int screenDpi = 0;
};

QSize nativeSizeOf(const QSize& logicalSize, qreal devicePixelRatio);

// False while the native window and its Qt screen disagree on DPI. Windows
// defers WM_DPICHANGED for hidden windows, so the two drift apart after a
// monitor change. Sizes reported in that state are not trustworthy.
bool scaleIsSettled(int nativeDpi, int screenDpi);
bool scaleIsSettled(const WindowScaleState& state);

// True when a resize is one the app asked for: the scale is settled, nothing
// changed since the snapshot, and the reported size agrees with the current one.
bool resizeIsAuthoritative(const WindowScaleSnapshot& snapshot, const WindowScaleState& state);

// The size to put back, or invalid when nothing needs doing. Fires only once
// the scale settled and the window drifted away from the snapshot.
QSize sizeToRestore(const WindowScaleSnapshot& snapshot, const WindowScaleState& state);

// True when QWindow::resize(target) would be a no-op because the current size
// already matches, but the last reported size does not. The window then needs
// a one pixel nudge to get a fresh resize event.
bool needsNudge(const QSize& target, const WindowScaleState& state);

} // namespace mf
