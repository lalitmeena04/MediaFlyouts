#pragma once

#include <QObject>
#include <QString>

class QSystemTrayIcon;
class QMenu;
class QTimer;
class Settings;

// Own notification-area icon and menu. Emit QML app intents.
class TrayController : public QObject {
        Q_OBJECT

    public:
        explicit TrayController(const QString& iconFamily, Settings* settings, QObject* parent = nullptr);
        ~TrayController() override;

        void show();
        // Show balloon notification from tray icon.
        void notify(const QString& title, const QString& message);

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
        // Delay single-click flyout. Double-click can open dashboard.
        QTimer* m_singleClickTimer = nullptr;
};
