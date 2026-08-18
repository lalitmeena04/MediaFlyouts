#pragma once

#include <QObject>
#include <QString>

class QSystemTrayIcon;
class QMenu;
class QTimer;
class Settings;

// Owns the notification-area icon and its menu; emits intents the QML app root reacts to.
class TrayController : public QObject {
        Q_OBJECT

    public:
        explicit TrayController(const QString& iconFamily, Settings* settings, QObject* parent = nullptr);
        ~TrayController() override;

        void show();

    signals:
        void openDashboardRequested();
        void showFlyoutRequested();
        void openMixerRequested();

    private:
        void applyVisibility();

        QSystemTrayIcon* m_tray = nullptr;
        QMenu* m_menu = nullptr;
        QString m_iconFamily;
        Settings* m_settings = nullptr;
        // defers the single-click flyout so a double-click can open the dashboard instead
        QTimer* m_singleClickTimer = nullptr;
};
