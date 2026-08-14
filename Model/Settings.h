#pragma once

#include <QObject>
#include <QSettings>
#include <QUrl>

// Persistent user settings, stored per-user (MSIX-safe) via QSettings. Exposed to QML.
class Settings : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool mediaFlyoutEnabled READ mediaFlyoutEnabled WRITE setMediaFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool seekbarEnabled READ seekbarEnabled WRITE setSeekbarEnabled NOTIFY changed)
        Q_PROPERTY(bool mediaFlyoutAlwaysDisplay READ mediaFlyoutAlwaysDisplay WRITE setMediaFlyoutAlwaysDisplay NOTIFY
                       changed)
        Q_PROPERTY(bool centerTitleArtist READ centerTitleArtist WRITE setCenterTitleArtist NOTIFY changed)
        Q_PROPERTY(bool compactLayout READ compactLayout WRITE setCompactLayout NOTIFY changed)
        Q_PROPERTY(bool volumeFlyoutEnabled READ volumeFlyoutEnabled WRITE setVolumeFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool overrideNativeOsd READ overrideNativeOsd WRITE setOverrideNativeOsd NOTIFY changed)
        Q_PROPERTY(bool lockKeysEnabled READ lockKeysEnabled WRITE setLockKeysEnabled NOTIFY changed)
        Q_PROPERTY(bool nextUpEnabled READ nextUpEnabled WRITE setNextUpEnabled NOTIFY changed)
        Q_PROPERTY(bool disableOnFullscreen READ disableOnFullscreen WRITE setDisableOnFullscreen NOTIFY changed)
        Q_PROPERTY(bool launchOnStartup READ launchOnStartup WRITE setLaunchOnStartup NOTIFY changed)
        Q_PROPERTY(bool trayIconHidden READ trayIconHidden WRITE setTrayIconHidden NOTIFY changed)
        Q_PROPERTY(bool trayLeftClickShowsFlyout READ trayLeftClickShowsFlyout WRITE setTrayLeftClickShowsFlyout NOTIFY
                       changed)
        Q_PROPERTY(bool taskbarWidgetEnabled READ taskbarWidgetEnabled WRITE setTaskbarWidgetEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetControlsEnabled READ taskbarWidgetControlsEnabled WRITE
                       setTaskbarWidgetControlsEnabled NOTIFY changed)
        Q_PROPERTY(int flyoutPosition READ flyoutPosition WRITE setFlyoutPosition NOTIFY changed)
        Q_PROPERTY(int flyoutMonitor READ flyoutMonitor WRITE setFlyoutMonitor NOTIFY changed)
        Q_PROPERTY(int flyoutDuration READ flyoutDuration WRITE setFlyoutDuration NOTIFY changed)
        Q_PROPERTY(int flyoutAnimationSpeed READ flyoutAnimationSpeed WRITE setFlyoutAnimationSpeed NOTIFY changed)
        Q_PROPERTY(int flyoutAnimationMs READ flyoutAnimationMs NOTIFY changed)
        Q_PROPERTY(int volumeFlyoutDuration READ volumeFlyoutDuration WRITE setVolumeFlyoutDuration NOTIFY changed)
        Q_PROPERTY(int nextUpDuration READ nextUpDuration WRITE setNextUpDuration NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetOffset READ taskbarWidgetOffset WRITE setTaskbarWidgetOffset NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetSide READ taskbarWidgetSide WRITE setTaskbarWidgetSide NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetMonitor READ taskbarWidgetMonitor WRITE setTaskbarWidgetMonitor NOTIFY changed)

    public:
        explicit Settings(QObject* parent = nullptr);

        // write / read every setting to a portable ini file (for backup & restore)
        Q_INVOKABLE bool exportTo(const QUrl& file) const;
        Q_INVOKABLE bool importFrom(const QUrl& file);

        bool mediaFlyoutEnabled() const;
        bool seekbarEnabled() const;
        bool mediaFlyoutAlwaysDisplay() const;
        bool centerTitleArtist() const;
        bool compactLayout() const;
        bool volumeFlyoutEnabled() const;
        bool overrideNativeOsd() const;
        bool lockKeysEnabled() const;
        bool nextUpEnabled() const;
        bool disableOnFullscreen() const;
        bool launchOnStartup() const;
        bool trayIconHidden() const;
        bool trayLeftClickShowsFlyout() const;
        bool taskbarWidgetEnabled() const;
        bool taskbarWidgetControlsEnabled() const;
        int flyoutPosition() const;
        int flyoutMonitor() const;
        int flyoutDuration() const;
        int flyoutAnimationSpeed() const;
        // fade duration in ms derived from flyoutAnimationSpeed
        int flyoutAnimationMs() const;
        int volumeFlyoutDuration() const;
        int nextUpDuration() const;
        int taskbarWidgetOffset() const;
        int taskbarWidgetSide() const;
        int taskbarWidgetMonitor() const;

        void setMediaFlyoutEnabled(bool v);
        void setSeekbarEnabled(bool v);
        void setMediaFlyoutAlwaysDisplay(bool v);
        void setCenterTitleArtist(bool v);
        void setCompactLayout(bool v);
        void setVolumeFlyoutEnabled(bool v);
        void setOverrideNativeOsd(bool v);
        void setLockKeysEnabled(bool v);
        void setNextUpEnabled(bool v);
        void setDisableOnFullscreen(bool v);
        void setLaunchOnStartup(bool v);
        void setTrayIconHidden(bool v);
        void setTrayLeftClickShowsFlyout(bool v);
        void setTaskbarWidgetEnabled(bool v);
        void setTaskbarWidgetControlsEnabled(bool v);
        void setFlyoutPosition(int v);
        void setFlyoutMonitor(int v);
        void setFlyoutDuration(int v);
        void setFlyoutAnimationSpeed(int v);
        void setVolumeFlyoutDuration(int v);
        void setNextUpDuration(int v);
        void setTaskbarWidgetOffset(int v);
        void setTaskbarWidgetSide(int v);
        void setTaskbarWidgetMonitor(int v);

    signals:
        void changed();

    private:
        void assign(const char* key, bool& field, bool v);
        void assignInt(const char* key, int& field, int v);
        void reload();

        QSettings m_store;
        bool m_mediaFlyoutEnabled;
        bool m_seekbarEnabled;
        bool m_mediaFlyoutAlwaysDisplay;
        bool m_centerTitleArtist;
        bool m_compactLayout;
        bool m_volumeFlyoutEnabled;
        bool m_overrideNativeOsd;
        bool m_lockKeysEnabled;
        bool m_nextUpEnabled;
        bool m_disableOnFullscreen;
        bool m_launchOnStartup;
        bool m_trayIconHidden;
        bool m_trayLeftClickShowsFlyout;
        bool m_taskbarWidgetEnabled;
        bool m_taskbarWidgetControlsEnabled;
        int m_flyoutPosition;
        int m_flyoutMonitor;
        int m_flyoutDuration;
        int m_flyoutAnimationSpeed;
        int m_volumeFlyoutDuration;
        int m_nextUpDuration;
        int m_taskbarWidgetOffset;
        int m_taskbarWidgetSide;
        int m_taskbarWidgetMonitor;
};
