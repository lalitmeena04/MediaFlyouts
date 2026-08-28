# Model API

The `Model/` sources build into the `mediaflyout_core` static library
(`Qt6::Core` only) and hold the app's portable, unit-tested domain logic.

## Settings
`Settings : QObject` — persisted application settings backed by `QSettings`,
exposed to QML as the `settings` context property.
- One `Q_PROPERTY` per setting with a `changed()` notify signal.
- `exportTo(QUrl)` / `importFrom(QUrl)` — backup and restore.
- `playerAllowed(name)`, `filterMode`, `filterList`, `addFilterEntry` /
  `removeFilterEntry` — app filtering.
- `configDir()` — the settings folder (for "Open Config Folder").
- Most defaults are literals in `reload()`; `taskbarWidgetSide` is the exception
  — on first run it is seeded from the system taskbar alignment and written to
  the store, so a later system change never moves a widget the user has placed.

## TrackInfo (`namespace mf`)
Plain struct describing the current track (title, artist, album, source app,
`PlaybackState`, position/duration, repeat/shuffle, capability flags) plus
helpers: `formatDuration(ms)`, `formatTrackLine(track)`,
`playbackProgress(track)`.

## PlaybackController
`IPlaybackController` — transport abstraction (`play`, `pause`, `next`,
`previous`, optional `seek` / `setRepeatMode` / `setShuffle` / session cycling)
so playback logic stays mockable. `togglePlayPause(controller, state)` is a free
helper.

## Fft
`mf::fft`, `mf::magnitudeSpectrum`, `mf::spectrumBands`, and `mf::frequencyBars`
— a radix-2 FFT and log-spaced, dB-mapped band extraction used by the visualizer.

## FlyoutPositioner
Pure geometry that computes a flyout's on-screen rectangle from a position enum,
screen bounds and size — the testable core behind `FlyoutPlacement`.

## AlbumArtProvider
A `QQuickImageProvider` that serves the current album art to QML (`image://…`).

## WinMediaProvider *(Windows)*
Bridges the WinRT **System Media Transport Controls** to `TrackInfo`, feeding
`NowPlaying`.
