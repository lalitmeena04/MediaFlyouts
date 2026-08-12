#include "TrayController.h"

#include <QAction>
#include <QApplication>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSystemTrayIcon>

namespace {

// renders a Fluent glyph to a tray-sized icon so we don't need a separate PNG asset
QIcon renderGlyphIcon(const QString& family, QChar glyph) {
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    if(!family.isEmpty()) {
        QPainter p(&pm);
        QFont f(family);
        f.setPixelSize(26);
        p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(pm.rect(), Qt::AlignCenter, QString(glyph));
    }
    return QIcon(pm);
}

} // namespace

TrayController::TrayController(const QString& iconFamily, QObject* parent) : QObject(parent), m_iconFamily(iconFamily) {
    m_menu = new QMenu();
    QAction* dashAction = m_menu->addAction(QStringLiteral("Dashboard"));
    QAction* flyoutAction = m_menu->addAction(QStringLiteral("Show Media Flyout"));
    m_menu->addSeparator();
    QAction* quitAction = m_menu->addAction(QStringLiteral("Quit MediaFlyouts"));

    connect(dashAction, &QAction::triggered, this, &TrayController::openDashboardRequested);
    connect(flyoutAction, &QAction::triggered, this, &TrayController::showFlyoutRequested);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_tray = new QSystemTrayIcon(renderGlyphIcon(m_iconFamily, QChar(0xE863)), this);
    m_tray->setToolTip(QStringLiteral("MediaFlyouts"));
    m_tray->setContextMenu(m_menu);

    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if(reason == QSystemTrayIcon::Trigger)
            emit openDashboardRequested();
    });
}

TrayController::~TrayController() {
    delete m_menu;
}

void TrayController::show() {
    m_tray->show();
}
