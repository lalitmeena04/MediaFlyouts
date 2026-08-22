# User Guide

MediaFlyouts lives in the system tray and shows overlays in response to media,
volume and lock keys. This guide covers day-to-day use and the settings
dashboard.

## The tray icon

Right-click the tray icon for the menu:

- **Settings** — open the dashboard.
- **Show Media Flyout** — pop the media flyout for the current track.
- **Volume Mixer** — open the standalone per-app mixer.
- **Repository / Report a Bug** — open the project on GitHub.
- **Open Config Folder** — open the folder holding your settings file.
- **Quit** — exit the app.

Left-click behavior (open Settings vs. show the flyout) is configurable; with the
"show flyout" option, a double-click opens Settings.

## Flyouts

### Media flyout
Press a media key (or a volume key, unless excluded) while a player is active to
show album art, title, artist and transport controls. The **seekbar** is
draggable; the pin/close control keeps it on screen ("always display") until you
dismiss it.

### Volume flyout
Press a volume key to show the master volume slider (it replaces the native
Windows OSD). Click the **chevron** to expand the **per-app mixer**, where each
app has its own volume slider, mute toggle and icon. Changes apply immediately.

### Lock-keys flyout
Toggling Caps / Num / Scroll / Insert shows a brief indicator. Enable or disable
each key, the duration, and the bold style in Settings.

### Up next
When a track auto-advances, a compact flyout shows what started playing. Choose
the **card** or **compact pill** style in Settings.

## Taskbar widget

Enable the taskbar widget to show the current track next to the tray. You can:

- dock it to the **left or right**, move it between **monitors**, and nudge it
  with an **offset**;
- **scroll** or **swipe** across it to switch between multiple playing apps;
- turn on the **audio visualizer** and pick a style — **Bars**, **Radial**,
  **Waveform** or **Particles**.

The widget hides automatically over fullscreen apps and stays out of screen
captures.

## The settings dashboard

Open it from the tray menu. The sidebar groups settings into pages (Media Flyout,
Volume, Lock Keys, Taskbar Widget, App Filtering, About, …). Use the **search
box** to jump to any setting. Highlights:

- **Theme & accent** — light/dark/system, and a default / match-system / custom
  accent (or tint it from album art).
- **Placement & animation** — flyout position, target monitor, duration,
  animation speed and easing.
- **App filtering** — blacklist or whitelist which players may trigger flyouts.
- **Backup / restore** — export and import all settings.

## Startup & updates

Enable **launch on startup** to have MediaFlyouts start with Windows (minimized
to tray). Turn on **automatic update checks** to be notified when a newer release
is available; you can also check manually from the About page.
