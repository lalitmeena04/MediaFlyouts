# Changelog

All notable changes to MediaFlyouts are documented here. This project follows
[Semantic Versioning](https://semver.org/).

## [1.0.0] — 2026-08-27

First public release. A complete media / volume / lock-key overlay for Windows,
written from scratch in Qt 6 / C++ / QML.

### Media flyout
- Media flyout on media / volume keys via the System Media Transport Controls,
  with album art, title, artist and optional player name.
- Previous / play-pause / next transport controls.
- Draggable interactive seekbar with elapsed / duration, plus a seekbar toggle
  and interactive sub-option.
- Repeat and shuffle controls when the session supports them.
- Compact layout, centered text, and an "always display" pinned mode with a
  close button that auto-hides when unpinned.
- Rounded album art.

### Volume flyout & mixer
- Volume flyout that replaces and suppresses the native Windows volume OSD.
- Master volume slider with mute and a configurable duration.
- Follows the default output device and re-binds on device changes.
- Expandable, responsive per-app mixer inside the flyout, with per-app icons.
- Standalone Volume Mixer window with per-process volume / mute.

### Lock-keys flyout
- Caps / Num / Scroll / Insert indicators with per-key toggles, a configurable
  duration and a bold-UI option.

### Now Playing / Up next
- Fires on track change; selectable "now playing card" or compact "Up next"
  pill style; sizes to its track text; configurable duration.

### Taskbar widget & visualizer
- Now-playing taskbar widget: left/right docking, per-monitor placement,
  adjustable offset, center alignment, pause overlay and scrolling titles.
- Scroll / swipe between multiple media sources; hidden over fullscreen apps and
  kept out of screen captures; optional integrated controls.
- Audio visualizer from WASAPI loopback + FFT with **Bars**, **Radial**,
  **Waveform** and **Particles** (beat-driven) styles and mirrored bars.

### Flyouts, theming & placement
- Simultaneously visible flyouts stack and re-flow smoothly.
- Configurable animation speed and easing.
- Six positions plus "above the taskbar widget", monitor selection and
  on-screen clamping.
- Light / dark / system theme with dark title bars.
- Selectable accent color (default / match-system / custom) and optional accent
  tinted from album art.

### App filtering, dashboard & system
- Blacklist / whitelist app filtering.
- Searchable settings dashboard with scrollable pages and settings
  backup / restore.
- Tray icon with a dark Fluent context menu; hide-tray and left-click behavior
  options; open dashboard on double-click.
- Single-instance handling that surfaces the dashboard on relaunch.
- First-run onboarding wizard.
- Update checker against the GitHub releases API with an optional automatic
  check and tray notification.
- Launch on startup and fullscreen-app suppression.

### Engineering
- Cross-platform domain core (settings, track info, FFT, positioning) with
  GoogleTest + Qt Test suites.
- GitHub Actions CI building and testing both debug and release, running on
  Node 24 action runtimes.

[1.0.0]: https://github.com/scleaverzer0ne/MediaFlyouts/releases/tag/v1.0.0
