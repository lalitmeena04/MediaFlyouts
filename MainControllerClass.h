#pragma once

#include <memory>

#include <QString>
#include <QtGlobal>

class QApplication;
class QQmlApplicationEngine;

class SingleInstance;
class NowPlaying;
class Settings;
class MonitorMapper;
class FlyoutPlacement;
class WindowScaleRefresher;
class SwipeDetector;
class ThemeController;
class UpdateChecker;
class ErrorReporter;
class Capabilities;
class QmlLogger;
class AlbumArtProvider;
class TrayController;

#ifdef Q_OS_WIN
class WinMediaProvider;
class VolumeController;
class NativeFlyoutSuppressor;
class FullscreenDetector;
class LockKeysController;
class StartupController;
class TaskbarInfo;
class VolumeMixer;
class AudioCapture;
class WindowEffects;
class AccentWatcher;
class MediaKeyController;
class BatteryStatusController;
class PeripheralAlerts;
class PowerStatusController;
#endif

class MainControllerClass {
    public:
        explicit MainControllerClass(QApplication& app);
        ~MainControllerClass();

        int run();

    private:
        void loadIconFont();
        void buildCoreSubsystems();
        void registerContextProperties();
#ifdef Q_OS_WIN
        void buildPlatformSubsystems();
#endif

        QApplication& m_app;
        bool m_primary = false;
        QString m_iconFamily;

        // Construction order also sets teardown order.
        std::unique_ptr<SingleInstance> m_instance;
        std::unique_ptr<NowPlaying> m_nowPlaying;
        std::unique_ptr<Settings> m_settings;
        std::unique_ptr<MonitorMapper> m_monitors;
        std::unique_ptr<FlyoutPlacement> m_placement;
        std::unique_ptr<WindowScaleRefresher> m_scaleRefresher;
        std::unique_ptr<SwipeDetector> m_swipe;
        std::unique_ptr<ThemeController> m_theme;
        std::unique_ptr<UpdateChecker> m_updateChecker;
        std::unique_ptr<Capabilities> m_capabilities;
        std::unique_ptr<ErrorReporter> m_errorReporter;
        std::unique_ptr<QQmlApplicationEngine> m_engine;
        std::unique_ptr<QmlLogger> m_qmlLog;
        std::unique_ptr<TrayController> m_tray;
        AlbumArtProvider* m_artProvider = nullptr; // Owned by QML engine.

#ifdef Q_OS_WIN
        std::unique_ptr<WinMediaProvider> m_provider;
        std::unique_ptr<VolumeController> m_volumeController;
        std::unique_ptr<NativeFlyoutSuppressor> m_nativeSuppressor;
        std::unique_ptr<FullscreenDetector> m_fullscreen;
        std::unique_ptr<LockKeysController> m_lockKeys;
        std::unique_ptr<StartupController> m_startup;
        std::unique_ptr<TaskbarInfo> m_taskbar;
        std::unique_ptr<VolumeMixer> m_volumeMixer;
        std::unique_ptr<AudioCapture> m_audioCapture;
        std::unique_ptr<WindowEffects> m_windowEffects;
        std::unique_ptr<AccentWatcher> m_accentWatcher;
        std::unique_ptr<MediaKeyController> m_mediaKeys;
        std::unique_ptr<BatteryStatusController> m_batteryStatus;
        std::unique_ptr<PeripheralAlerts> m_peripheralAlerts;
        std::unique_ptr<PowerStatusController> m_powerStatus;
#endif
};
