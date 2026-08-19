#include "TrayController.h"

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>

#include "Settings.h"

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

TrayController::TrayController(const QString& iconFamily, Settings* settings, QObject* parent)
    : QObject(parent), m_iconFamily(iconFamily), m_settings(settings) {
    m_menu = new QMenu();
    QAction* dashAction = m_menu->addAction(QStringLiteral("Dashboard"));
    QAction* flyoutAction = m_menu->addAction(QStringLiteral("Show Media Flyout"));
    QAction* mixerAction = m_menu->addAction(QStringLiteral("Volume Mixer"));
    QAction* configAction = m_menu->addAction(QStringLiteral("Open Config Folder"));
    m_menu->addSeparator();
    QAction* quitAction = m_menu->addAction(QStringLiteral("Quit MediaFlyouts"));

    connect(dashAction, &QAction::triggered, this, &TrayController::openDashboardRequested);
    connect(flyoutAction, &QAction::triggered, this, &TrayController::showFlyoutRequested);
    connect(mixerAction, &QAction::triggered, this, &TrayController::openMixerRequested);
    connect(configAction, &QAction::triggered, this, [this] {
        if(m_settings)
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_settings->configDir()));
    });
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_tray = new QSystemTrayIcon(renderGlyphIcon(m_iconFamily, QChar(0xE863)), this);
    m_tray->setToolTip(QStringLiteral("MediaFlyouts"));
    m_tray->setContextMenu(m_menu);

    // in flyout mode a single click is held briefly so a double-click can open the dashboard instead
    m_singleClickTimer = new QTimer(this);
    m_singleClickTimer->setSingleShot(true);
    m_singleClickTimer->setInterval(QApplication::doubleClickInterval());
    connect(m_singleClickTimer, &QTimer::timeout, this, &TrayController::showFlyoutRequested);

    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        const bool flyoutMode = m_settings && m_settings->trayLeftClickShowsFlyout();
        if(reason == QSystemTrayIcon::DoubleClick) {
            // cancel the pending single-click flyout and show the dashboard
            if(flyoutMode) {
                m_singleClickTimer->stop();
                emit openDashboardRequested();
            }
            return;
        }
        if(reason != QSystemTrayIcon::Trigger)
            return;
        if(flyoutMode)
            m_singleClickTimer->start();
        else
            emit openDashboardRequested();
    });

    if(m_settings)
        connect(m_settings, &Settings::changed, this, &TrayController::applyVisibility);
}

void TrayController::applyVisibility() {
    if(m_tray)
        m_tray->setVisible(!(m_settings && m_settings->trayIconHidden()));
}

TrayController::~TrayController() {
    delete m_menu;
}

void TrayController::show() {
    applyVisibility();
}

void TrayController::notify(const QString& title, const QString& message) {
    if(m_tray)
        m_tray->showMessage(title, message);
}
