# MediaFlyouts — Features

MediaFlyouts is a Windows media, volume and lock-key overlay built with
Qt 6 / C++ / QML. This document describes the features that ship in the app.

> Anything not listed here is not implemented yet. In particular there is **no
> window acrylic/mica backdrop** (flyouts use solid, theme-matched surfaces with
> dark title bars), **no localization** (English only for now) and **no
> telemetry**.

## Media flyout

- Appears on media-key or volume-key presses while something is playing, driven
  by the System Media Transport Controls (SMTC).
- Shows album art (rounded), title, artist and an optional media-player name.
- Transport controls: previous, play/pause, next.
- Draggable, interactive seekbar with elapsed / duration, plus an optional
  "interactive" sub-mode; the seekbar can be turned off entirely.
- Optional repeat and shuffle controls (shown when the session supports them).
- Compact layout, centered title/artist, and an "always display" pinned mode
  with a close button that auto-hides once unpinned.

## Volume flyout & per-app mixer

- Replaces the native Windows volume OSD (the system OSD is suppressed).
- Master volume slider with mute, and a configurable stay duration.
- Follows the default output device and re-binds automatically when the default
  device changes.
- Expandable per-app mixer (chevron) directly inside the flyout, with a live,
  responsive slider and mute per application and the real app icon.
- A standalone Volume Mixer window offering the same per-process controls.

## Lock-keys flyout

- Indicators for Caps Lock, Num Lock, Scroll Lock and Insert.
- Per-key enable toggles, a configurable stay duration and a bold-UI option.

## Now Playing / Up next flyout

- Pops when the track changes so it reflects the newly started track.
- Two user-selectable styles: a "now playing" card or a compact "Up next:" pill.
- Sizes itself to the track text; configurable stay duration.

## Taskbar widget

- A compact now-playing widget docked next to the system tray.
- Dock to the left or right side, move between monitors, and nudge with an
  offset that can shift either direction.
- Center alignment, a pause overlay, and marquee scrolling for long titles.
- Scroll wheel / swipe to switch between multiple simultaneous media sources,
  with source indicator dots.
- Hidden automatically over fullscreen apps and kept out of screen captures.
- Optional integrated playback controls.

## Taskbar visualizer

- Real-time audio spectrum from WASAPI loopback capture with an FFT, mapped to
  log-spaced frequency bands.
- Selectable styles: **Bars**, **Radial**, **Waveform** and **Particles**
  (the particle burst is driven by a bass beat detector).
- Mirrored / centered bars option; colored by the current accent.

## Flyout stacking, animation & placement

- Multiple simultaneously visible flyouts stack above each other and re-flow
  smoothly when one disappears.
- Configurable animation speed and easing style.
- Six on-screen positions plus a "dock above the taskbar widget" option, a
  monitor selector, and clamping so above-widget flyouts stay on screen.

## Theming & accent

- Light / Dark / System theme, with theme-matched (dark) window title bars.
- Selectable accent color: default, match the Windows system accent, or a custom
  color.
- Optional accent tint derived from the current album art.

## App filtering

- Blacklist or whitelist mode to control which players may trigger flyouts,
  with a per-player allow check.

## Dashboard (settings)

- Sidebar navigation with scrollable pages and a settings search box.
- Backup and restore of all settings via export / import.

## System tray & single instance

- Tray icon with a dark, Fluent-style context menu (Settings, Show Media
  Flyout, Volume Mixer, Repository, Report a Bug, Open Config Folder, Quit),
  each with an icon.
- Option to hide the tray icon and to choose the left-click behavior; the
  dashboard opens on double-click when single-click is set to show the flyout.
- Single-instance handling: a second launch surfaces the running instance's
  dashboard and exits.

## Onboarding & updates

- First-run onboarding wizard.
- Update checker against the GitHub releases API, with an optional automatic
  check and a tray notification when an update is available.

## System integration

- Launch on startup via the Windows `Run` registry key.
- Fullscreen / exclusive-app detection to suppress flyouts during full-screen
  games and apps.

## Windows APIs used

- **Media:** `GlobalSystemMediaTransportControlsSessionManager` (WinRT SMTC).
- **Volume:** Core Audio — `IMMDeviceEnumerator`, `IAudioEndpointVolume`,
  `IAudioSessionManager2`, `ISimpleAudioVolume`, `IMMNotificationClient`.
- **Keys:** low-level keyboard hook (`WH_KEYBOARD_LL`), `GetKeyState`.
- **Audio capture:** WASAPI loopback (`IAudioCaptureClient`) + a radix-2 FFT.
- **Title bars:** `DwmSetWindowAttribute` (dark frame).
- **Fullscreen detection:** `SHQueryUserNotificationState`.
- **Accent:** WinRT `UISettings` accent color.
- **Startup:** Windows `Run` registry key.
