#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

class Settings : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool mediaFlyoutEnabled READ mediaFlyoutEnabled WRITE setMediaFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool seekbarEnabled READ seekbarEnabled WRITE setSeekbarEnabled NOTIFY changed)
        Q_PROPERTY(bool seekbarInteractive READ seekbarInteractive WRITE setSeekbarInteractive NOTIFY changed)
        Q_PROPERTY(bool mediaFlyoutAlwaysDisplay READ mediaFlyoutAlwaysDisplay WRITE setMediaFlyoutAlwaysDisplay NOTIFY
                       changed)
        Q_PROPERTY(bool centerTitleArtist READ centerTitleArtist WRITE setCenterTitleArtist NOTIFY changed)
        Q_PROPERTY(bool compactLayout READ compactLayout WRITE setCompactLayout NOTIFY changed)
        Q_PROPERTY(bool playerInfoEnabled READ playerInfoEnabled WRITE setPlayerInfoEnabled NOTIFY changed)
        Q_PROPERTY(bool repeatEnabled READ repeatEnabled WRITE setRepeatEnabled NOTIFY changed)
        Q_PROPERTY(bool shuffleEnabled READ shuffleEnabled WRITE setShuffleEnabled NOTIFY changed)
        Q_PROPERTY(bool volumeFlyoutEnabled READ volumeFlyoutEnabled WRITE setVolumeFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool volumeFlyoutMixerEnabled READ volumeFlyoutMixerEnabled WRITE setVolumeFlyoutMixerEnabled NOTIFY
                       changed)
        Q_PROPERTY(bool overrideNativeOsd READ overrideNativeOsd WRITE setOverrideNativeOsd NOTIFY changed)
        Q_PROPERTY(bool lockKeysEnabled READ lockKeysEnabled WRITE setLockKeysEnabled NOTIFY changed)
        Q_PROPERTY(bool lockKeysCapsEnabled READ lockKeysCapsEnabled WRITE setLockKeysCapsEnabled NOTIFY changed)
        Q_PROPERTY(bool lockKeysNumEnabled READ lockKeysNumEnabled WRITE setLockKeysNumEnabled NOTIFY changed)
        Q_PROPERTY(bool lockKeysScrollEnabled READ lockKeysScrollEnabled WRITE setLockKeysScrollEnabled NOTIFY changed)
        Q_PROPERTY(bool lockKeysInsertEnabled READ lockKeysInsertEnabled WRITE setLockKeysInsertEnabled NOTIFY changed)
        Q_PROPERTY(bool lockKeysBoldUi READ lockKeysBoldUi WRITE setLockKeysBoldUi NOTIFY changed)
        Q_PROPERTY(int lockKeysDuration READ lockKeysDuration WRITE setLockKeysDuration NOTIFY changed)
        Q_PROPERTY(bool nextUpEnabled READ nextUpEnabled WRITE setNextUpEnabled NOTIFY changed)
        Q_PROPERTY(bool disableOnFullscreen READ disableOnFullscreen WRITE setDisableOnFullscreen NOTIFY changed)
        Q_PROPERTY(bool flyoutSwipeDismissEnabled READ flyoutSwipeDismissEnabled WRITE setFlyoutSwipeDismissEnabled NOTIFY
                       changed)
        Q_PROPERTY(bool launchOnStartup READ launchOnStartup WRITE setLaunchOnStartup NOTIFY changed)
        Q_PROPERTY(bool trayIconHidden READ trayIconHidden WRITE setTrayIconHidden NOTIFY changed)
        Q_PROPERTY(bool trayLeftClickShowsFlyout READ trayLeftClickShowsFlyout WRITE setTrayLeftClickShowsFlyout NOTIFY
                       changed)
        Q_PROPERTY(bool taskbarWidgetEnabled READ taskbarWidgetEnabled WRITE setTaskbarWidgetEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetControlsEnabled READ taskbarWidgetControlsEnabled WRITE
                       setTaskbarWidgetControlsEnabled NOTIFY changed)
        Q_PROPERTY(int flyoutPosition READ flyoutPosition WRITE setFlyoutPosition NOTIFY changed)
        Q_PROPERTY(int flyoutMonitor READ flyoutMonitor WRITE setFlyoutMonitor NOTIFY changed)
        Q_PROPERTY(int volumeFlyoutPosition READ volumeFlyoutPosition WRITE setVolumeFlyoutPosition NOTIFY changed)
        Q_PROPERTY(int volumeFlyoutMonitor READ volumeFlyoutMonitor WRITE setVolumeFlyoutMonitor NOTIFY changed)
        Q_PROPERTY(int flyoutDuration READ flyoutDuration WRITE setFlyoutDuration NOTIFY changed)
        Q_PROPERTY(int flyoutAnimationSpeed READ flyoutAnimationSpeed WRITE setFlyoutAnimationSpeed NOTIFY changed)
        Q_PROPERTY(int flyoutAnimationMs READ flyoutAnimationMs NOTIFY changed)
        Q_PROPERTY(int flyoutAnimationEasingStyle READ flyoutAnimationEasingStyle WRITE setFlyoutAnimationEasingStyle
                       NOTIFY changed)
        Q_PROPERTY(int volumeFlyoutDuration READ volumeFlyoutDuration WRITE setVolumeFlyoutDuration NOTIFY changed)
        Q_PROPERTY(int nextUpDuration READ nextUpDuration WRITE setNextUpDuration NOTIFY changed)
        Q_PROPERTY(int nextUpStyle READ nextUpStyle WRITE setNextUpStyle NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetOffset READ taskbarWidgetOffset WRITE setTaskbarWidgetOffset NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetSide READ taskbarWidgetSide WRITE setTaskbarWidgetSide NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetMonitor READ taskbarWidgetMonitor WRITE setTaskbarWidgetMonitor NOTIFY changed)
        Q_PROPERTY(int taskbarWidgetLayout READ taskbarWidgetLayout WRITE setTaskbarWidgetLayout NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetShowPauseOverlay READ taskbarWidgetShowPauseOverlay WRITE
                       setTaskbarWidgetShowPauseOverlay NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetScrollingEnabled READ taskbarWidgetScrollingEnabled WRITE
                       setTaskbarWidgetScrollingEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetAdaptiveColors READ taskbarWidgetAdaptiveColors WRITE
                       setTaskbarWidgetAdaptiveColors NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetPauseAllEnabled READ taskbarWidgetPauseAllEnabled WRITE
                       setTaskbarWidgetPauseAllEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarWidgetPlayAllEnabled READ taskbarWidgetPlayAllEnabled WRITE
                       setTaskbarWidgetPlayAllEnabled NOTIFY changed)
        Q_PROPERTY(bool taskbarVisualizerEnabled READ taskbarVisualizerEnabled WRITE setTaskbarVisualizerEnabled NOTIFY
                       changed)
        Q_PROPERTY(bool taskbarVisualizerCentered READ taskbarVisualizerCentered WRITE setTaskbarVisualizerCentered
                       NOTIFY changed)
        Q_PROPERTY(int visualizerStyle READ visualizerStyle WRITE setVisualizerStyle NOTIFY changed)
        Q_PROPERTY(bool peripheralsWidgetEnabled READ peripheralsWidgetEnabled WRITE setPeripheralsWidgetEnabled
                       NOTIFY changed)
        Q_PROPERTY(bool peripheralsWidgetWiredEnabled READ peripheralsWidgetWiredEnabled WRITE
                       setPeripheralsWidgetWiredEnabled NOTIFY changed)
        Q_PROPERTY(QStringList peripheralsWidgetExcluded READ peripheralsWidgetExcluded NOTIFY changed)
        Q_PROPERTY(QVariantMap peripheralNames READ peripheralNames NOTIFY changed)
        Q_PROPERTY(bool batteryRingUseAccent READ batteryRingUseAccent WRITE setBatteryRingUseAccent NOTIFY changed)
        Q_PROPERTY(bool peripheralFlyoutEnabled READ peripheralFlyoutEnabled WRITE setPeripheralFlyoutEnabled
                       NOTIFY changed)
        Q_PROPERTY(int peripheralFlyoutDuration READ peripheralFlyoutDuration WRITE setPeripheralFlyoutDuration
                       NOTIFY changed)
        Q_PROPERTY(int peripheralFlyoutPosition READ peripheralFlyoutPosition WRITE setPeripheralFlyoutPosition
                       NOTIFY changed)
        Q_PROPERTY(int peripheralFlyoutMonitor READ peripheralFlyoutMonitor WRITE setPeripheralFlyoutMonitor
                       NOTIFY changed)
        Q_PROPERTY(bool peripheralFlyoutOnConnect READ peripheralFlyoutOnConnect WRITE setPeripheralFlyoutOnConnect
                       NOTIFY changed)
        Q_PROPERTY(bool peripheralFlyoutOnDisconnect READ peripheralFlyoutOnDisconnect WRITE
                       setPeripheralFlyoutOnDisconnect NOTIFY changed)
        Q_PROPERTY(bool peripheralFlyoutOnLowBattery READ peripheralFlyoutOnLowBattery WRITE
                       setPeripheralFlyoutOnLowBattery NOTIFY changed)
        Q_PROPERTY(bool peripheralFlyoutOnCharging READ peripheralFlyoutOnCharging WRITE
                       setPeripheralFlyoutOnCharging NOTIFY changed)
        Q_PROPERTY(bool peripheralFlyoutOnWired READ peripheralFlyoutOnWired WRITE setPeripheralFlyoutOnWired
                       NOTIFY changed)
        Q_PROPERTY(bool peripheralLowBatteryToast READ peripheralLowBatteryToast WRITE setPeripheralLowBatteryToast
                       NOTIFY changed)
        Q_PROPERTY(bool chargerFlyoutEnabled READ chargerFlyoutEnabled WRITE setChargerFlyoutEnabled NOTIFY changed)
        Q_PROPERTY(bool chargerFlyoutOnPlug READ chargerFlyoutOnPlug WRITE setChargerFlyoutOnPlug NOTIFY changed)
        Q_PROPERTY(bool chargerFlyoutOnUnplug READ chargerFlyoutOnUnplug WRITE setChargerFlyoutOnUnplug NOTIFY changed)
        Q_PROPERTY(int chargerFlyoutDuration READ chargerFlyoutDuration WRITE setChargerFlyoutDuration NOTIFY changed)
        Q_PROPERTY(int chargerFlyoutPosition READ chargerFlyoutPosition WRITE setChargerFlyoutPosition NOTIFY changed)
        Q_PROPERTY(int chargerFlyoutMonitor READ chargerFlyoutMonitor WRITE setChargerFlyoutMonitor NOTIFY changed)
        Q_PROPERTY(bool onboardingCompleted READ onboardingCompleted WRITE setOnboardingCompleted NOTIFY changed)
        Q_PROPERTY(bool accentFromArt READ accentFromArt WRITE setAccentFromArt NOTIFY changed)
        Q_PROPERTY(bool accentFromArtAppWide READ accentFromArtAppWide WRITE setAccentFromArtAppWide NOTIFY changed)
        Q_PROPERTY(int accentMode READ accentMode WRITE setAccentMode NOTIFY changed)
        Q_PROPERTY(QString customAccent READ customAccent WRITE setCustomAccent NOTIFY changed)
        Q_PROPERTY(int appTheme READ appTheme WRITE setAppTheme NOTIFY changed)
        Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY changed)
        Q_PROPERTY(QStringList filterList READ filterList NOTIFY changed)
        Q_PROPERTY(bool autoCheckUpdates READ autoCheckUpdates WRITE setAutoCheckUpdates NOTIFY changed)
        Q_PROPERTY(QString skippedUpdateVersion READ skippedUpdateVersion WRITE setSkippedUpdateVersion NOTIFY changed)
        // UI language code such as "de" or "pt_BR". Empty follows the system language.
        Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY changed)

    public:
        explicit Settings(QObject* parent = nullptr);

        Q_INVOKABLE bool exportTo(const QUrl& file) const;
        Q_INVOKABLE bool importFrom(const QUrl& file);
        // Drops every stored value and reloads the defaults. Onboarding stays completed.
        Q_INVOKABLE void resetToDefaults();
        Q_INVOKABLE QString configDir() const;

        bool mediaFlyoutEnabled() const;
        bool seekbarEnabled() const;
        bool seekbarInteractive() const;
        bool mediaFlyoutAlwaysDisplay() const;
        bool centerTitleArtist() const;
        bool compactLayout() const;
        bool playerInfoEnabled() const;
        bool repeatEnabled() const;
        bool shuffleEnabled() const;
        bool volumeFlyoutEnabled() const;
        bool volumeFlyoutMixerEnabled() const;
        bool overrideNativeOsd() const;
        bool lockKeysEnabled() const;
        bool lockKeysCapsEnabled() const;
        bool lockKeysNumEnabled() const;
        bool lockKeysScrollEnabled() const;
        bool lockKeysInsertEnabled() const;
        bool lockKeysBoldUi() const;
        int lockKeysDuration() const;
        bool nextUpEnabled() const;
        bool disableOnFullscreen() const;
        bool flyoutSwipeDismissEnabled() const;
        bool launchOnStartup() const;
        bool trayIconHidden() const;
        bool trayLeftClickShowsFlyout() const;
        bool taskbarWidgetEnabled() const;
        bool taskbarWidgetControlsEnabled() const;
        int flyoutPosition() const;
        int flyoutMonitor() const;
        int volumeFlyoutPosition() const;
        int volumeFlyoutMonitor() const;
        int flyoutDuration() const;
        int flyoutAnimationSpeed() const;
        int flyoutAnimationMs() const;
        int flyoutAnimationEasingStyle() const;
        int volumeFlyoutDuration() const;
        int nextUpDuration() const;
        int nextUpStyle() const;
        int taskbarWidgetOffset() const;
        int taskbarWidgetSide() const;
        int taskbarWidgetMonitor() const;
        int taskbarWidgetLayout() const;
        bool taskbarWidgetShowPauseOverlay() const;
        bool taskbarWidgetScrollingEnabled() const;
        bool taskbarWidgetAdaptiveColors() const;
        bool taskbarWidgetPauseAllEnabled() const;
        bool taskbarWidgetPlayAllEnabled() const;
        bool taskbarVisualizerEnabled() const;
        bool taskbarVisualizerCentered() const;
        int visualizerStyle() const;
        bool peripheralsWidgetEnabled() const;
        bool peripheralsWidgetWiredEnabled() const;
        QStringList peripheralsWidgetExcluded() const;
        Q_INVOKABLE bool peripheralSelected(const QString& name) const;
        Q_INVOKABLE void setPeripheralSelected(const QString& name, bool selected);
        QVariantMap peripheralNames() const;
        Q_INVOKABLE QString peripheralDisplayName(const QString& name) const;
        Q_INVOKABLE void setPeripheralDisplayName(const QString& name, const QString& custom);
        bool batteryRingUseAccent() const;
        bool peripheralFlyoutEnabled() const;
        int peripheralFlyoutDuration() const;
        int peripheralFlyoutPosition() const;
        int peripheralFlyoutMonitor() const;
        bool peripheralFlyoutOnConnect() const;
        bool peripheralFlyoutOnDisconnect() const;
        bool peripheralFlyoutOnLowBattery() const;
        bool peripheralFlyoutOnCharging() const;
        bool peripheralFlyoutOnWired() const;
        bool peripheralLowBatteryToast() const;
        bool chargerFlyoutEnabled() const;
        bool chargerFlyoutOnPlug() const;
        bool chargerFlyoutOnUnplug() const;
        int chargerFlyoutDuration() const;
        int chargerFlyoutPosition() const;
        int chargerFlyoutMonitor() const;
        bool onboardingCompleted() const;
        bool accentFromArt() const;
        bool accentFromArtAppWide() const;
        int accentMode() const;
        QString customAccent() const;
        int appTheme() const;
        int filterMode() const;
        QStringList filterList() const;
        bool autoCheckUpdates() const;
        QString skippedUpdateVersion() const;
        QString language() const;
        Q_INVOKABLE bool playerAllowed(const QString& player) const;

        void setMediaFlyoutEnabled(bool v);
        void setSeekbarEnabled(bool v);
        void setSeekbarInteractive(bool v);
        void setMediaFlyoutAlwaysDisplay(bool v);
        void setCenterTitleArtist(bool v);
        void setCompactLayout(bool v);
        void setPlayerInfoEnabled(bool v);
        void setRepeatEnabled(bool v);
        void setShuffleEnabled(bool v);
        void setVolumeFlyoutEnabled(bool v);
        void setVolumeFlyoutMixerEnabled(bool v);
        void setOverrideNativeOsd(bool v);
        void setLockKeysEnabled(bool v);
        void setLockKeysCapsEnabled(bool v);
        void setLockKeysNumEnabled(bool v);
        void setLockKeysScrollEnabled(bool v);
        void setLockKeysInsertEnabled(bool v);
        void setLockKeysBoldUi(bool v);
        void setLockKeysDuration(int v);
        void setNextUpEnabled(bool v);
        void setDisableOnFullscreen(bool v);
        void setFlyoutSwipeDismissEnabled(bool v);
        void setLaunchOnStartup(bool v);
        void setTrayIconHidden(bool v);
        void setTrayLeftClickShowsFlyout(bool v);
        void setTaskbarWidgetEnabled(bool v);
        void setTaskbarWidgetControlsEnabled(bool v);
        void setFlyoutPosition(int v);
        void setFlyoutMonitor(int v);
        void setVolumeFlyoutPosition(int v);
        void setVolumeFlyoutMonitor(int v);
        void setFlyoutDuration(int v);
        void setFlyoutAnimationSpeed(int v);
        void setFlyoutAnimationEasingStyle(int v);
        void setVolumeFlyoutDuration(int v);
        void setNextUpDuration(int v);
        void setNextUpStyle(int v);
        void setTaskbarWidgetOffset(int v);
        void setTaskbarWidgetSide(int v);
        void setTaskbarWidgetMonitor(int v);
        void setTaskbarWidgetLayout(int v);
        void setTaskbarWidgetShowPauseOverlay(bool v);
        void setTaskbarWidgetScrollingEnabled(bool v);
        void setTaskbarWidgetAdaptiveColors(bool v);
        void setTaskbarWidgetPauseAllEnabled(bool v);
        void setTaskbarWidgetPlayAllEnabled(bool v);
        void setTaskbarVisualizerEnabled(bool v);
        void setTaskbarVisualizerCentered(bool v);
        void setVisualizerStyle(int v);
        void setPeripheralsWidgetEnabled(bool v);
        void setPeripheralsWidgetWiredEnabled(bool v);
        void setBatteryRingUseAccent(bool v);
        void setPeripheralFlyoutEnabled(bool v);
        void setPeripheralFlyoutDuration(int v);
        void setPeripheralFlyoutPosition(int v);
        void setPeripheralFlyoutMonitor(int v);
        void setPeripheralFlyoutOnConnect(bool v);
        void setPeripheralFlyoutOnDisconnect(bool v);
        void setPeripheralFlyoutOnLowBattery(bool v);
        void setPeripheralFlyoutOnCharging(bool v);
        void setPeripheralFlyoutOnWired(bool v);
        void setPeripheralLowBatteryToast(bool v);
        void setChargerFlyoutEnabled(bool v);
        void setChargerFlyoutOnPlug(bool v);
        void setChargerFlyoutOnUnplug(bool v);
        void setChargerFlyoutDuration(int v);
        void setChargerFlyoutPosition(int v);
        void setChargerFlyoutMonitor(int v);
        void setOnboardingCompleted(bool v);
        void setAccentFromArt(bool v);
        void setAccentFromArtAppWide(bool v);
        void setAccentMode(int v);
        void setCustomAccent(const QString& v);
        void setAppTheme(int v);
        void setFilterMode(int v);
        void setAutoCheckUpdates(bool v);
        void setSkippedUpdateVersion(const QString& v);
        void setLanguage(const QString& v);
        Q_INVOKABLE void addFilterEntry(const QString& name);
        Q_INVOKABLE void removeFilterEntry(const QString& name);

    signals:
        void changed();

    private:
        void assign(const char* key, bool& field, bool v);
        void assignInt(const char* key, int& field, int v);
        void reload();

        QSettings m_store;
        bool m_mediaFlyoutEnabled;
        bool m_seekbarEnabled;
        bool m_seekbarInteractive;
        bool m_mediaFlyoutAlwaysDisplay;
        bool m_centerTitleArtist;
        bool m_compactLayout;
        bool m_playerInfoEnabled;
        bool m_repeatEnabled;
        bool m_shuffleEnabled;
        bool m_volumeFlyoutEnabled;
        bool m_volumeFlyoutMixerEnabled;
        bool m_overrideNativeOsd;
        bool m_lockKeysEnabled;
        bool m_lockKeysCapsEnabled;
        bool m_lockKeysNumEnabled;
        bool m_lockKeysScrollEnabled;
        bool m_lockKeysInsertEnabled;
        bool m_lockKeysBoldUi;
        int m_lockKeysDuration;
        bool m_nextUpEnabled;
        bool m_disableOnFullscreen;
        bool m_flyoutSwipeDismissEnabled;
        bool m_launchOnStartup;
        bool m_trayIconHidden;
        bool m_trayLeftClickShowsFlyout;
        bool m_taskbarWidgetEnabled;
        bool m_taskbarWidgetControlsEnabled;
        int m_flyoutPosition;
        int m_flyoutMonitor;
        int m_volumeFlyoutPosition;
        int m_volumeFlyoutMonitor;
        int m_flyoutDuration;
        int m_flyoutAnimationSpeed;
        int m_flyoutAnimationEasingStyle;
        int m_volumeFlyoutDuration;
        int m_nextUpDuration;
        int m_nextUpStyle;
        int m_taskbarWidgetOffset;
        int m_taskbarWidgetSide;
        int m_taskbarWidgetMonitor;
        int m_taskbarWidgetLayout;
        bool m_taskbarWidgetShowPauseOverlay;
        bool m_taskbarWidgetScrollingEnabled;
        bool m_taskbarWidgetAdaptiveColors;
        bool m_taskbarWidgetPauseAllEnabled;
        bool m_taskbarWidgetPlayAllEnabled;
        bool m_taskbarVisualizerEnabled;
        bool m_taskbarVisualizerCentered;
        int m_visualizerStyle;
        bool m_peripheralsWidgetEnabled;
        bool m_peripheralsWidgetWiredEnabled;
        QStringList m_peripheralsWidgetExcluded;
        QVariantMap m_peripheralNames;
        bool m_batteryRingUseAccent;
        bool m_peripheralFlyoutEnabled;
        int m_peripheralFlyoutDuration;
        int m_peripheralFlyoutPosition;
        int m_peripheralFlyoutMonitor;
        bool m_peripheralFlyoutOnConnect;
        bool m_peripheralFlyoutOnDisconnect;
        bool m_peripheralFlyoutOnLowBattery;
        bool m_peripheralFlyoutOnCharging;
        bool m_peripheralFlyoutOnWired;
        bool m_peripheralLowBatteryToast;
        bool m_chargerFlyoutEnabled;
        bool m_chargerFlyoutOnPlug;
        bool m_chargerFlyoutOnUnplug;
        int m_chargerFlyoutDuration;
        int m_chargerFlyoutPosition;
        int m_chargerFlyoutMonitor;
        bool m_onboardingCompleted;
        bool m_accentFromArt;
        bool m_accentFromArtAppWide;
        int m_accentMode;
        QString m_customAccent;
        int m_appTheme;
        int m_filterMode;
        QStringList m_filterList;
        bool m_autoCheckUpdates;
        QString m_skippedUpdateVersion;
        QString m_language;
};
