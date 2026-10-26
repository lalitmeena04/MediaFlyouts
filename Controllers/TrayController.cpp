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

#include "Capabilities.h"
#include "InstallSource.h"
#include "Logging.h"
#include "Settings.h"

namespace {

// Shell small-icon size runs 16 px at 100 % up to 64 px at 400 %.
constexpr int kIconSizes[] = {16, 20, 24, 32, 48, 64};
constexpr QChar kTrayGlyph(0xE863);

QIcon renderGlyphIcon(const QString& family, QChar glyph) {
    QIcon icon;
    for(int size : kIconSizes) {
        QPixmap pm(size, size);
        pm.fill(Qt::transparent);
        if(!family.isEmpty()) {
            QPainter p(&pm);
            QFont f(family);
            // Glyph fills about 80 % of the cell. Same ratio as the old 26/32.
            f.setPixelSize(qMax(1, size * 26 / 32));
            p.setFont(f);
            p.setPen(Qt::white);
            p.drawText(pm.rect(), Qt::AlignCenter, QString(glyph));
        }
        icon.addPixmap(pm);
    }
    return icon;
}

} // namespace

TrayController::TrayController(const QString& iconFamily, Settings* settings, QObject* parent)
    : QObject(parent), m_iconFamily(iconFamily), m_settings(settings) {
    m_menu = new QMenu();
    m_menu->setStyleSheet(QStringLiteral(
        "QMenu { background-color: #2b2b2b; color: #ffffff; border: 1px solid #3d3d3d; border-radius: 8px; "
        "padding: 6px; }"
        "QMenu::item { padding: 8px 34px 8px 12px; border-radius: 6px; margin: 2px 4px; }"
        "QMenu::item:selected { background-color: #3a3a3a; }"
        "QMenu::icon { padding-left: 10px; }"
        "QMenu::separator { height: 1px; background: #3d3d3d; margin: 6px 10px; }"));

    QAction* dashAction = m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xF6B3)), QStringLiteral("Settings"));
    QAction* flyoutAction =
        m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xF610)), QStringLiteral("Show Media Flyout"));
    QAction* mixerAction =
        m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xEB4C)), QStringLiteral("Volume Mixer"));
    m_menu->addSeparator();
    QAction* repoAction = m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xF1FB)), QStringLiteral("Repository"));
    QAction* bugAction =
        m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xE21B)), QStringLiteral("Report a Bug"));
    QAction* configAction =
        m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xF41D)), QStringLiteral("Open Config Folder"));
    m_menu->addSeparator();
    QAction* restartAction =
        m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xF13E)), QStringLiteral("Restart MediaFlyouts"));
    QAction* quitAction =
        m_menu->addAction(renderGlyphIcon(m_iconFamily, QChar(0xF619)), QStringLiteral("Quit MediaFlyouts"));

    connect(dashAction, &QAction::triggered, this, &TrayController::openDashboardRequested);
    connect(flyoutAction, &QAction::triggered, this, &TrayController::showFlyoutRequested);
    connect(mixerAction, &QAction::triggered, this, &TrayController::openMixerRequested);
    connect(repoAction, &QAction::triggered, this,
            [] { QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/scleaverzer0ne/MediaFlyouts"))); });
    connect(bugAction, &QAction::triggered, this, &TrayController::reportBugRequested);
    connect(configAction, &QAction::triggered, this, [this] {
        if(!m_settings)
            return;
        // Packaged installs keep the folder inside the package cache.
        const QString dir = mf::hostVisiblePath(m_settings->configDir());
        TRC_INFO("tray: open config folder {}", dir.toStdString());
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });
    connect(restartAction, &QAction::triggered, this, &TrayController::restartRequested);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_tray = new QSystemTrayIcon(renderGlyphIcon(m_iconFamily, kTrayGlyph), this);
    m_tray->setToolTip(QStringLiteral("MediaFlyouts"));
    m_tray->setContextMenu(m_menu);

    m_singleClickTimer = new QTimer(this);
    m_singleClickTimer->setSingleShot(true);
    m_singleClickTimer->setInterval(QApplication::doubleClickInterval());
    connect(m_singleClickTimer, &QTimer::timeout, this, &TrayController::showFlyoutRequested);

    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        const bool flyoutMode = m_settings && m_settings->trayLeftClickShowsFlyout();
        if(reason == QSystemTrayIcon::DoubleClick) {
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

void TrayController::trackCapabilities(Capabilities* capabilities) {
    m_capabilities = capabilities;
    if(!m_capabilities)
        return;
    connect(m_capabilities, &Capabilities::changed, this, &TrayController::applyToolTip);
    applyToolTip();
}

void TrayController::applyToolTip() {
    if(m_tray && m_capabilities)
        m_tray->setToolTip(m_capabilities->statusSummary());
}

TrayController::~TrayController() {
    delete m_menu;
}

void TrayController::show() {
    applyVisibility();
}

void TrayController::refreshIcon() {
    if(!m_tray)
        return;
    TRC_DEBUG("tray: re-applying icon after display change");
    m_tray->setIcon(renderGlyphIcon(m_iconFamily, kTrayGlyph));
}

void TrayController::notify(const QString& title, const QString& message) {
    if(m_tray)
        m_tray->showMessage(title, message);
}
