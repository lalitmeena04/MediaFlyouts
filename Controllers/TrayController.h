#pragma once

#include <QObject>
#include <QString>

class QSystemTrayIcon;
class QMenu;
class QTimer;
class Settings;
class Capabilities;

// Own notification-area icon and menu. Emit QML app intents.
class TrayController : public QObject {
        Q_OBJECT

    public:
        explicit TrayController(const QString& iconFamily, Settings* settings, QObject* parent = nullptr);
        ~TrayController() override;

        void show();
        // Show balloon notification from tray icon.
        void notify(const QString& title, const QString& message);
        // Report unavailable subsystems in the tray tooltip.
        void trackCapabilities(Capabilities* capabilities);

    signals:
        void openDashboardRequested();
        void showFlyoutRequested();
        void openMixerRequested();
        // Raised by the tray "Report a Bug" entry. Opens the report page.
        void reportBugRequested();

    private:
        void applyVisibility();
        void applyToolTip();

        QSystemTrayIcon* m_tray = nullptr;
        QMenu* m_menu = nullptr;
        QString m_iconFamily;
        Settings* m_settings = nullptr;
        Capabilities* m_capabilities = nullptr;
        // Delay single-click flyout. Double-click can open dashboard.
        QTimer* m_singleClickTimer = nullptr;
};
