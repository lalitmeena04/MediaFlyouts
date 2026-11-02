#pragma once

#include <QList>
#include <QObject>
#include <QString>

class QAction;
class QSystemTrayIcon;
class QMenu;
class QTimer;
class Settings;
class Capabilities;

class TrayController : public QObject {
        Q_OBJECT

    public:
        explicit TrayController(const QString& iconFamily, Settings* settings, QObject* parent = nullptr);
        ~TrayController() override;

        void show();
        void notify(const QString& title, const QString& message);
        void trackCapabilities(Capabilities* capabilities);

    public slots:
        void refreshIcon();
        // Re-applies menu and tooltip text after a language switch.
        void retranslate();

    signals:
        void openDashboardRequested();
        void showFlyoutRequested();
        void openMixerRequested();
        void reportBugRequested();
        void restartRequested();

    private:
        void applyVisibility();
        void applyToolTip();

        QSystemTrayIcon* m_tray = nullptr;
        QMenu* m_menu = nullptr;
        QString m_iconFamily;
        Settings* m_settings = nullptr;
        Capabilities* m_capabilities = nullptr;
        QTimer* m_singleClickTimer = nullptr;
        QList<QAction*> m_actions;
};
