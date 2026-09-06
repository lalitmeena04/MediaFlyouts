#include "FlyoutPlacement.h"

#include <QGuiApplication>
#include <QScreen>

#include "FlyoutPositioner.h"
#include "Settings.h"

FlyoutPlacement::FlyoutPlacement(Settings* settings, QObject* parent) : QObject(parent), m_settings(settings) {}

QPoint FlyoutPlacement::position(int width, int height) const {
    return positionAt(width, height, m_settings ? m_settings->flyoutPosition() : int(mf::FlyoutPosition::BottomCenter),
                      m_settings ? m_settings->flyoutMonitor() : 0);
}

QPoint FlyoutPlacement::positionAt(int width, int height, int positionIndex, int monitor) const {
    const QList<QScreen*> screens = QGuiApplication::screens();
    if(screens.isEmpty())
        return QPoint(0, 0);

    if(monitor < 0 || monitor >= screens.size())
        monitor = 0;

    // Screen availableGeometry excludes taskbar. Use small uniform margin.
    const QRect work = screens.at(monitor)->availableGeometry();
    const auto pos = mf::FlyoutPosition(qBound(0, positionIndex, int(mf::FlyoutPosition::BottomRight)));
    return mf::computeFlyoutRect(work, QSize(width, height), pos, 24).topLeft();
}

QStringList FlyoutPlacement::monitorNames() const {
    QStringList names;
    const QList<QScreen*> screens = QGuiApplication::screens();
    for(int i = 0; i < screens.size(); ++i) {
        const QScreen* s = screens.at(i);
        names << QStringLiteral("%1. %2 (%3x%4)")
                     .arg(i + 1)
                     .arg(s->name())
                     .arg(s->geometry().width())
                     .arg(s->geometry().height());
    }
    return names;
}
