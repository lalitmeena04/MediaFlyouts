#pragma once

#include <QObject>
#include <QString>

class QSystemTrayIcon;
class QMenu;
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

    private:
        void applyVisibility();

        QSystemTrayIcon* m_tray = nullptr;
        QMenu* m_menu = nullptr;
        QString m_iconFamily;
        Settings* m_settings = nullptr;
};
