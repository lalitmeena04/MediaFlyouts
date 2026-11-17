import QtQuick
import MediaFlyouts

// Stage "parts". Applies demo state, creates one real view, lets it settle,
// grabs it, then moves on. Views stay hidden: onboardingCompleted is false in
// the scratch settings, so no flyout presents itself and no widget docks.
QtObject {
    id: director

    property Component mediaFlyout: Component { MediaFlyout {} }
    property Component nowPlayingFlyout: Component { NowPlayingFlyout {} }
    property Component volumeFlyout: Component { VolumeFlyout {} }
    property Component lockKeysFlyout: Component { LockKeysFlyout {} }
    property Component peripheralFlyout: Component { PeripheralFlyout {} }
    property Component taskbarWidget: Component { TaskbarWidget {} }
    property Component peripheralsWidget: Component { PeripheralsWidget {} }

    // Invented tracks. Covers are generated, see DemoArt.cpp.
    readonly property var tracks: [
        { "title": "Neon Tide", "artist": "Lumen Arc", "player": "Music", "cover": 0, "position": 72, "duration": 225 },
        { "title": "Paper Lanterns", "artist": "Kiri Sound", "player": "Browser", "cover": 1, "position": 141, "duration": 198, "shuffle": true },
        { "title": "Glass Harbor", "artist": "Tidewater Radio", "player": "Music", "cover": 2, "position": 33, "duration": 254 },
        { "title": "Slow Orbit", "artist": "Vela & Rune", "player": "Music", "cover": 3, "position": 95, "duration": 212, "repeat": 2 },
        { "title": "Saffron Skies", "artist": "Mira Kole", "player": "Browser", "cover": 4, "position": 12, "duration": 187 },
        { "title": "Afterglow Drive", "artist": "Station North", "player": "Music", "cover": 5, "position": 160, "duration": 241 }
    ]

    readonly property var baseSettings: ({
        "appTheme": 2, "accentMode": 2, "accentFromArt": true, "accentFromArtAppWide": true,
        "compactLayout": false, "seekbarEnabled": true, "seekbarInteractive": true, "playerInfoEnabled": true,
        "shuffleEnabled": true, "repeatEnabled": true, "centerTitleArtist": false,
        "mediaFlyoutAlwaysDisplay": false, "visualizerStyle": 0, "taskbarVisualizerCentered": true,
        "peripheralsWidgetWiredEnabled": true
    })

    readonly property var devices: [
        { "name": "Wireless Mouse", "percent": 84, "connectionKind": "wireless", "category": "mouse", "charging": false, "components": [] },
        { "name": "Mechanical Keyboard", "percent": 61, "connectionKind": "wireless", "category": "keyboard", "charging": false, "components": [] },
        { "name": "Studio Headphones", "percent": 72, "connectionKind": "bluetooth", "category": "", "charging": false, "components": [] },
        { "name": "Game Controller", "percent": 38, "connectionKind": "bluetooth", "category": "gamepad", "charging": true, "components": [] }
    ]

    readonly property var alerts: [
        { "event": "connected", "name": "Wireless Mouse", "percent": 84, "bucket": "Good", "category": "mouse", "connectionKind": "wireless", "charging": false },
        { "event": "chargingStarted", "name": "Studio Headphones", "percent": 56, "bucket": "Good", "category": "", "connectionKind": "bluetooth", "charging": true },
        { "event": "lowBattery", "name": "Game Controller", "percent": 18, "bucket": "Low", "category": "gamepad", "connectionKind": "bluetooth", "charging": false }
    ]

    function mixerSessions() {
        return [
            { "index": 0, "name": "Music", "icon": showcase.appIconUrl(0).toString(), "volume": 78, "muted": false },
            { "index": 1, "name": "Browser", "icon": showcase.appIconUrl(1).toString(), "volume": 45, "muted": false },
            { "index": 2, "name": "Game", "icon": showcase.appIconUrl(2).toString(), "volume": 100, "muted": false },
            { "index": 3, "name": "Voice Chat", "icon": showcase.appIconUrl(3).toString(), "volume": 64, "muted": false },
            { "index": 4, "name": "System Sounds", "icon": "", "volume": 30, "muted": true }
        ]
    }

    // Band levels shaped like quiet music: stronger lows, fading highs. Kept
    // low because the widget draws its title over the visualizer.
    function spectrum(seed, count) {
        var bands = []
        for(var i = 0; i < count; i++) {
            var t = i / (count - 1)
            var envelope = 0.74 - 0.30 * t
            var ripple = 0.5 + 0.5 * Math.sin(i * 1.7 + seed * 2.3) * Math.cos(i * 0.45 + seed)
            bands.push(Math.max(0.08, Math.min(1, envelope * (0.3 + 0.7 * ripple))))
        }
        return bands
    }

    // name: output file. view: component. track: index into tracks, -1 for none.
    // settings / pro: overrides. props: initial view properties. setup: extra state.
    readonly property var parts: [
        { "name": "media-a", "view": director.mediaFlyout, "track": 0 },
        { "name": "media-b", "view": director.mediaFlyout, "track": 1 },
        { "name": "media-c", "view": director.mediaFlyout, "track": 2 },
        { "name": "media-d", "view": director.mediaFlyout, "track": 3 },
        { "name": "media-compact", "view": director.mediaFlyout, "track": 4, "settings": { "compactLayout": true } },
        { "name": "media-light", "view": director.mediaFlyout, "track": 5, "settings": { "appTheme": 1 } },
        { "name": "nowplaying-card", "view": director.nowPlayingFlyout, "track": 1 },
        { "name": "nowplaying-pill", "view": director.nowPlayingFlyout, "track": 2, "pro": { "nextUpStyle": 1 } },
        { "name": "nowplaying-pill-b", "view": director.nowPlayingFlyout, "track": 5, "pro": { "nextUpStyle": 1 } },
        { "name": "volume", "view": director.volumeFlyout, "track": -1 },
        { "name": "volume-mixer", "view": director.volumeFlyout, "track": -1, "props": { "expanded": true },
          "setup": function() { volumeMixer.sessions = director.mixerSessions() } },
        { "name": "lock-caps", "view": director.lockKeysFlyout, "track": 0,
          "setup": function() { director.lockKey(0, "Caps Lock", true) } },
        { "name": "lock-num", "view": director.lockKeysFlyout, "track": 2,
          "setup": function() { director.lockKey(1, "Num Lock", true) } },
        { "name": "lock-scroll", "view": director.lockKeysFlyout, "track": 4,
          "setup": function() { director.lockKey(2, "Scroll Lock", false) } },
        { "name": "peripheral-flyout", "view": director.peripheralFlyout, "track": -1, "props": { "alerts": director.alerts } },
        { "name": "widget-bars", "view": director.taskbarWidget, "track": 0, "sessions": 3, "bands": 1 },
        { "name": "widget-wave", "view": director.taskbarWidget, "track": 3, "bands": 2, "settings": { "visualizerStyle": 2 } },
        { "name": "widget-radial", "view": director.taskbarWidget, "track": 2, "bands": 3, "settings": { "visualizerStyle": 1 } },
        { "name": "widget-plain", "view": director.taskbarWidget, "track": 5, "pro": { "visualizerEnabled": false } },
        { "name": "peripherals-widget", "view": director.peripheralsWidget, "track": -1,
          "setup": function() { batteryStatus.devices = director.devices } }
    ]

    property int languageIndex: -1
    property int index: -1
    property var live: null

    // The real controller translates the key name in C++.
    function lockKey(type, name, on) {
        lockKeys.keyType = type
        lockKeys.keyName = showcase.translate("LockKeysController", name)
        lockKeys.on = on
    }

    function apply(part) {
        world.reset()
        var key
        for(key in baseSettings)
            settings[key] = baseSettings[key]
        for(key in (part.settings || {}))
            settings[key] = part.settings[key]
        for(key in (part.pro || {}))
            pro[key] = part.pro[key]
        audioCapture.bands = spectrum(part.bands || 0, 28)
        if(part.track >= 0) {
            var track = Object.assign({}, tracks[part.track])
            track.sessions = part.sessions || 1
            showcase.setTrack(track)
        } else {
            showcase.clearTrack()
        }
        if(part.setup)
            part.setup()
    }

    function next() {
        if(director.live) {
            director.live.destroy()
            director.live = null
        }
        director.index++
        if(director.languageIndex < 0 || director.index >= director.parts.length) {
            director.languageIndex++
            if(director.languageIndex >= showcase.languages.length) {
                showcase.finish()
                return
            }
            showcase.setLanguage(showcase.languages[director.languageIndex])
            director.index = 0
        }
        var part = director.parts[director.index]
        director.apply(part)
        director.live = part.view.createObject(null, part.props || {})
        if(!director.live || !showcase.prepare(director.live)) {
            showcase.fail("could not create part " + part.name)
            Qt.callLater(director.next)
            return
        }
        settle.restart()
    }

    // Album art loads asynchronously and a few properties animate in.
    property Timer settle: Timer {
        interval: 700
        onTriggered: {
            showcase.grabPart(director.live, director.parts[director.index].name)
            director.next()
        }
    }

    Component.onCompleted: Qt.callLater(director.next)
}
