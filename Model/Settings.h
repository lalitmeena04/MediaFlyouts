#pragma once

#include <QObject>
#include <QSettings>

// Persistent user settings, stored per-user (MSIX-safe) via QSettings. Exposed to QML.
class Settings : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool mediaFlyoutEnabled READ mediaFlyoutEnabled WRITE setMediaFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool volumeFlyoutEnabled READ volumeFlyoutEnabled WRITE setVolumeFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool overrideNativeOsd READ overrideNativeOsd WRITE setOverrideNativeOsd NOTIFY changed)
        Q_PROPERTY(bool lockKeysEnabled READ lockKeysEnabled WRITE setLockKeysEnabled NOTIFY changed)
        Q_PROPERTY(bool nextUpEnabled READ nextUpEnabled WRITE setNextUpEnabled NOTIFY changed)
        Q_PROPERTY(bool disableOnFullscreen READ disableOnFullscreen WRITE setDisableOnFullscreen NOTIFY changed)
        Q_PROPERTY(bool launchOnStartup READ launchOnStartup WRITE setLaunchOnStartup NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetEnabled READ taskbarWidgetEnabled WRITE setTaskbarWidgetEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetControlsEnabled READ taskbarWidgetControlsEnabled WRITE
                       setTaskbarWidgetControlsEnabled NOTIFY changed)
        Q_PROPERTY(int flyoutPosition READ flyoutPosition WRITE setFlyoutPosition NOTIFY changed)
        Q_PROPERTY(int flyoutMonitor READ flyoutMonitor WRITE setFlyoutMonitor NOTIFY changed)
        Q_PROPERTY(int flyoutDuration READ flyoutDuration WRITE setFlyoutDuration NOTIFY changed)

    public:
        explicit Settings(QObject* parent = nullptr);

        bool mediaFlyoutEnabled() const;
        bool volumeFlyoutEnabled() const;
        bool overrideNativeOsd() const;
        bool lockKeysEnabled() const;
        bool nextUpEnabled() const;
        bool disableOnFullscreen() const;
        bool launchOnStartup() const;
        bool taskbarWidgetEnabled() const;
        bool taskbarWidgetControlsEnabled() const;
        int flyoutPosition() const;
        int flyoutMonitor() const;
        int flyoutDuration() const;

        void setMediaFlyoutEnabled(bool v);
        void setVolumeFlyoutEnabled(bool v);
        void setOverrideNativeOsd(bool v);
        void setLockKeysEnabled(bool v);
        void setNextUpEnabled(bool v);
        void setDisableOnFullscreen(bool v);
        void setLaunchOnStartup(bool v);
        void setTaskbarWidgetEnabled(bool v);
        void setTaskbarWidgetControlsEnabled(bool v);
        void setFlyoutPosition(int v);
        void setFlyoutMonitor(int v);
        void setFlyoutDuration(int v);

    signals:
        void changed();

    private:
        void assign(const char* key, bool& field, bool v);
        void assignInt(const char* key, int& field, int v);

        QSettings m_store;
        bool m_mediaFlyoutEnabled;
        bool m_volumeFlyoutEnabled;
        bool m_overrideNativeOsd;
        bool m_lockKeysEnabled;
        bool m_nextUpEnabled;
        bool m_disableOnFullscreen;
        bool m_launchOnStartup;
        bool m_taskbarWidgetEnabled;
        bool m_taskbarWidgetControlsEnabled;
        int m_flyoutPosition;
        int m_flyoutMonitor;
        int m_flyoutDuration;
};
