#include "FlyoutPlacement.h"

#include "FlyoutPositioner.h"
#include "MonitorMapper.h"
#include "Settings.h"

FlyoutPlacement::FlyoutPlacement(Settings* settings, const MonitorMapper* monitors, QObject* parent)
    : QObject(parent), m_settings(settings), m_monitors(monitors) {}

QPoint FlyoutPlacement::position(int width, int height) const {
    return positionAt(width, height, m_settings ? m_settings->flyoutPosition() : int(mf::FlyoutPosition::BottomCenter),
                      m_settings ? m_settings->flyoutMonitor() : 0);
}

QPoint FlyoutPlacement::positionAt(int width, int height, int positionIndex, int monitor) const {
    const MonitorEntry* entry = m_monitors ? m_monitors->entry(monitor) : nullptr;
    if(!entry)
        return QPoint(0, 0);

    // Available geometry excludes taskbar. Use small uniform margin.
    const auto pos = mf::FlyoutPosition(qBound(0, positionIndex, int(mf::FlyoutPosition::BottomRight)));
    return mf::computeFlyoutRect(entry->availableRect, QSize(width, height), pos, 24).topLeft();
}
