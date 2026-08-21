# MediaFlyouts — Features

This document captures the full feature set that MediaFlyouts aims to provide
in Qt 6 / C++ / QML. It is the source of truth for the story backlog in
[STORIES.md](STORIES.md).

## 1. Media Flyout (core)

- Master enable/disable toggle (`MediaFlyoutEnabled`).
- Appears when a media key or volume key (unless excluded) is pressed while media is playing.
- Displays album art thumbnail, track title, artist name, and optional media player name (`PlayerInfoEnabled`).
- Transport controls: Previous track, Play/Pause, Next track.
- Interactive seekbar with current position & duration display (updates every ~300ms when supported by session) (`SeekbarEnabled`).
- Optional Repeat and Shuffle playback controls (`RepeatEnabled`, `ShuffleEnabled`).
- Auto-hide duration control: Configurable display timeout (0–10,000 ms, default 3000 ms) and "Always display" pinned mode (`MediaFlyoutAlwaysDisplay`).
- Screen positioning: 6 preset positions (Bottom-Left, Bottom-Center, Bottom-Right, Top-Left, Top-Center, Top-Right) (`Position`).
- Target display selection for multi-monitor setups (`FlyoutSelectedMonitor`).
- Appearance & Backdrop: Acrylic backdrop toggle (`MediaFlyoutAcrylicWindowEnabled`), 4 background blur presets (`MediaFlyoutBackgroundBlur`), and customizable global acrylic opacity (0–255, default 175) (`AcrylicBlurOpacity`).
- Layout options: Compact layout mode (`CompactLayout`) and centered title/artist text alignment (`CenterTitleArtist`).
- Media session handling: "Pause other media automatically" option (`PauseOtherSessionsEnabled`) when focusing a new session.
- Exclude volume keys option (`MediaFlyoutVolumeKeysExcluded`) to suppress media flyout on volume key presses.

## 2. Volume Flyout & Volume Mixer

- Standalone Volume Flyout toggle (`VolumeControlEnabled`) replacing standard Windows Volume OSD.
- Master volume slider with configurable stay duration (0–10,000 ms, default 3000 ms).
- Option to position volume flyout directly above the Media Flyout (`VolumeControlAboveMediaFlyout`).
- Native Windows Volume OSD suppression (`HideVolumeOsd`) via UI Automation and Win32 window style manipulation.
- Standalone Volume Mixer window (`VolumeMixerEnabled`) with per-process volume sliders and mute toggles.
- Active audio application highlighting (`VolumeMixerHighlightActiveApps`) to identify processes currently outputting sound.
- Per-window acrylic blur toggle (`VolumeMixerAcrylicWindowEnabled`).

## 3. Lock Keys Flyout

- Master enable/disable toggle (`LockKeysEnabled`).
- Individual indicator toggles for Caps Lock, Num Lock, Scroll Lock, and Insert key (`LockKeysCapsEnabled`, `LockKeysNumEnabled`, `LockKeysScrollEnabled`, `LockKeysInsertEnabled`).
- Configurable stay duration (0–10,000 ms, default 2000 ms).
- Bold UI font styling option (`LockKeysBoldUi`).
- Smooth indicator animations for state changes (opacity, indicator width, shackle open/close rotation, bounce effect) (`LockKeysAnimated`).
- Target monitor preference: Default monitor (0), Monitor containing focused window (1), or Monitor containing mouse cursor (2) (`LockKeysMonitorPreference`).
- Per-window acrylic blur toggle (`LockKeysAcrylicWindowEnabled`).

## 4. Next Up Flyout

- Master enable/disable toggle (`NextUpEnabled`).
- Displays popup notification with track title, artist, and album art on auto-advance track transitions.
- Dynamic window width sizing based on string width calculations.
- Configurable stay duration (0–10,000 ms, default 2000 ms).
- Per-window acrylic blur toggle (`NextUpAcrylicWindowEnabled`).

## 5. Taskbar Widget

- Master enable/disable toggle (`TaskbarWidgetEnabled`).
- Embeds as a child window into Windows Taskbar (`Shell_TrayWnd`) using Win32 `SetParent` and UI Automation anchor detection.
- Target monitor selection for multi-monitor taskbars (`TaskbarWidgetSelectedMonitor`).
- Alignment choices: Left (0), Center (1), or Right (2) (`TaskbarWidgetPosition`).
- Taskbar padding: Native Windows Widgets button padding toggle (`TaskbarWidgetPadding`) and custom manual padding offset in pixels (`TaskbarWidgetManualPadding`).
- Legacy taskbar width calculation mode (`LegacyTaskbarWidthEnabled`) for compatibility with third-party taskbar mods (e.g., Nilesoft Shell, Windhawk).
- Win32 message filtering (`WM_GETOBJECT`, `WM_NCCALCSIZE`, `WM_IME_SETCONTEXT`) to prevent taskbar freezing issues.
- Background blur effect toggle (`TaskbarWidgetBackgroundBlur`).
- Auto-hide settings: Auto-hide after pause timeout (`TaskbarWidgetAutoHide`) and hide completely when no media session exists (`TaskbarWidgetHideCompletely`).
- Pause overlay icon toggle (`TaskbarWidgetShowPauseOverlay`) and widget animation toggle (`TaskbarWidgetAnimated`).
- Text behavior: Fixed width sizing (`TaskbarWidgetFixedWidth`), marquee scrolling for long titles (`TaskbarWidgetScrollingEnabled`), speed adjustment (1–100, `TaskbarWidgetScrollingTextSpeed`), endless loop mode (`TaskbarWidgetScrollingTextLoopForever`).
- Integrated playback controls (Previous, Play/Pause, Next) toggle (`TaskbarWidgetControlsEnabled`) and placement (Left or Right).

## 6. Taskbar Visualizer

- Master enable/disable toggle (`TaskbarVisualizerEnabled`).
- Real-time audio spectrum analyzer using WASAPI loopback audio capture and FFT processing.
- Positioning: Left (0) or Right (1) relative to the Taskbar Widget (`TaskbarVisualizerPosition`).
- Bar count customization (1–20 spectrum bars, default 10).
- Centered/mirrored spectrum bars toggle (`TaskbarVisualizerCenteredBars`).
- Spectrum baseline bar toggle (`TaskbarVisualizerBaseline`) with baseline auto-hide (`TaskbarVisualizerBaselineAutoHide`).
- Audio sensitivity adjustment (1–3, default 2) and peak level audio height calibration (1–3, default 3).
- Output audio device selector (`AudioDeviceMonitor`).
- Interactivity: Clickable visualizer region to directly open visualizer settings.

## 7. System Tray & Main Window (Settings)

- System tray icon with context menu: Settings, Repository link, View logs, Report bug, Quit.
- Configurable tray icon left-click behavior: Open Settings (0) or Show Media Flyout (1) (`NIconLeftClick`).
- Windows 11 symbol-style tray icon toggle (`NIconSymbol`) and option to hide tray icon completely (`NIconHide`).
- Single-instance process management: Named Mutex (`MediaFlyouts`) and IPC EventWaitHandle (`MediaFlyouts_OpenSettings`) to signal existing instance to show settings.
- Modern Settings Window with sidebar navigation pages: Home, Media Flyout, Volume, Taskbar Widget, Taskbar Visualizer, Next Up, Lock Keys, System, App Filtering, Advanced, About.
- Search across settings cards tagged for search index (`Tag="Indexable"`).

## 8. System, Appearance & Advanced

- Launch on system startup minimized to tray (`Startup`) via Windows Registry `Run` key or WinRT `startupTask`.
- Application theme mode: Windows Default (0), Light (1), Dark (2) (`AppTheme`).
- Dynamic accent color: Option to derive app accent color from current album art (`UseAlbumArtAsAccentColor`).
- Acrylic blur opacity slider (0–255, default 175) (`AcrylicBlurOpacity`).
- Global animation controls: Animation speed multiplier (Off, 0.5x, 1x, 1.5x, 2x, 3x) (`FlyoutAnimationSpeed`) and Easing style (Linear, Sine, Quadratic, Cubic) (`FlyoutAnimationEasingStyle`).
- Fullscreen app detection: DirectX exclusive fullscreen check using `SHQueryUserNotificationState` (`QUNS_RUNNING_D3D_FULL_SCREEN`) to automatically suppress flyouts during full-screen games/apps (`DisableIfFullscreen`).
- Configuration Backup & Restore: Export and import user settings via XML.
- Update checker & notifications: Periodic update checks, toast notifications (`ShowUpdateNotifications`), manual update check.
- Anonymous telemetry opt-in toggle (`AnonymousTelemetryAllowed`).

## 9. App Filtering

- Master enable/disable toggle (`AppFilteringEnabled`).
- Filtering mode toggle: Blacklist (0) or Whitelist (1) (`AppFilteringMode`).
- Allowed applications list (`AllowedApps`) and Blocked applications list (`BlockedApps`).
- Automatic discovery dropdown of currently running audio/media applications + manual executable name entry.

## 10. Onboarding & Localization

- First-run Onboarding Wizard window (`OnboardingWindow`): 4-step guided intro (Media Flyout, Volume Flyout, Lock Keys, Taskbar Widget/Features) with slide/fade page transitions.
- ~25–30 translation language dictionaries (e.g. ca, ar, cs, hi, he, fr, fi, es, en-US, de, it, id, hu, hr, ko, ja, nl, pl, pt-BR, zh-TW, zh-CN, vi, uk, tr, th, ta, sk, si, ru).
- Automatic system language detection and manual language selector (`AppLanguage`).
- Right-to-Left (RTL) layout support (`FlowDirection`) and font fallback strategy (`Segoe UI Variable, Microsoft YaHei UI, Yu Gothic UI`).

## Windows integration APIs used

- Media: `GlobalSystemMediaTransportControlsSessionManager` (WinRT SMTC).
- Volume: Core Audio APIs (`IMMDeviceEnumerator`, `IAudioSessionManager2`, `ISimpleAudioVolume`, `IAudioEndpointVolume`).
- Hotkeys/Keys: Low-level keyboard hook (`SetWindowsHookEx` - `WH_KEYBOARD_LL`) / `RegisterHotKey`, `GetKeyState`.
- Taskbar embedding: `SetParent` into `Shell_TrayWnd`, UI Automation (`AutomationElement`), Win32 message suppression (`WM_GETOBJECT`, `WM_NCCALCSIZE`, `WM_IME_SETCONTEXT`).
- Fullscreen detection: Shell notification API (`SHQueryUserNotificationState`).
- Single-instance IPC: Named `Mutex` and `EventWaitHandle`.
- Backdrop/blur: `DwmSetWindowAttribute` (Acrylic / Mica).
- Audio capture: WASAPI Loopback (`IAudioCaptureClient`) + FFT spectrum calculations.
- Startup: `windows.startupTask` (packaged) or Registry `Run` key (unpackaged).
