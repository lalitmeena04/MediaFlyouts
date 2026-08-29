# Controllers API

The `Controllers/` sources connect the model to Windows and to QML. Cross-platform
controllers build everywhere; the rest are Windows-only.

## Cross-platform

### NowPlaying
QML-facing now-playing model (`nowPlaying`). Applies `TrackInfo` updates, exposes
title/artist/art/progress and transport `Q_INVOKABLE`s, and decides whether a
change should raise the media flyout or the "up next" flyout. Forwards transport
commands to an `IPlaybackController`.

### TrayController
Owns the `QSystemTrayIcon` and its dark, Fluent-style context menu; emits intents
(open dashboard, show flyout, open mixer) and `notify()` toasts.

### ThemeController
Computes the theme/accent palette (`theme` context property) from `Settings` and
the system accent; honors light/dark/system and default/match-system/custom
accent.

### FlyoutPlacement
Resolves a flyout's screen position from the placement settings for QML
(`Q_INVOKABLE position(w, h)`, `monitorNames()`).

### SingleInstance
Ensures a single running instance (via a local server); a second launch signals
the primary to surface its dashboard.

### UpdateChecker
Queries the GitHub releases API and exposes `updateAvailable`, `latestVersion`,
etc. `isNewerVersion(current, latest)` does the dotted-numeric comparison. The
running version comes from `MEDIAFLYOUTS_VERSION`.

## Windows-only

- **VolumeController** — master volume/mute via `IAudioEndpointVolume`; follows
  the default output device (`IMMNotificationClient`).
- **VolumeMixer** — per-app sessions via `IAudioSessionManager2` /
  `ISimpleAudioVolume`, with process icons; exposes `sessions` to QML.
- **AudioCapture** — WASAPI loopback + FFT; exposes `bands`, `level` and a
  `beat()` signal for the visualizer.
- **AccentWatcher** — polls the WinRT `UISettings` accent color.
- **WindowEffects** — applies a dark title bar via `DwmSetWindowAttribute`.
- **LockKeysController / MediaKeyController** — low-level keyboard hook for lock
  keys and media keys.
- **FullscreenDetector** — suppresses flyouts over fullscreen/exclusive apps.
- **NativeFlyoutSuppressor** — hides the native Windows volume OSD. `main.cpp`
  only starts/arms it once onboarding is complete, so the system OSD keeps
  working during the first-run wizard.
- **StartupController** — launch-on-startup via the `Run` registry key.
- **TaskbarInfo** — taskbar geometry for placing the widget.
- **BatteryStatusController** — aggregates peripheral battery from all providers
  into one list for QML. Real providers poll on a worker thread; connect and
  disconnect events arriving mid-poll are coalesced rather than dropped.
- **BatteryProviders/** — one per transport: Bluetooth Classic (Hands-Free
  battery property), Bluetooth LE (GATT), Fast Pair (RFCOMM message stream),
  DualSense (raw HID), Razer (90-byte feature reports) and Logitech (HID++).
  `HidCollections` is the shared HID enumeration helper.
- **BluetoothConnectionWatcher / HidChangeWatcher** — instant connect and
  disconnect notifications, so the UI doesn't wait for the next poll.
- **PeripheralAlerts** — turns battery snapshots into connect / disconnect /
  low-battery events using `mf::BatteryAlertPolicy`.
