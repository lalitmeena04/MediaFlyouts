import QtQuick

// Stage "posters". Creates one scene at a time and saves it.
QtObject {
    id: director

    readonly property var posters: [
        { "name": "01-overview", "scene": "SceneOverview.qml" },
        { "name": "02-media-flyout", "scene": "SceneMedia.qml" },
        { "name": "03-taskbar-widget", "scene": "SceneWidget.qml" },
        { "name": "04-volume-mixer", "scene": "SceneMixer.qml" },
        { "name": "05-lock-keys", "scene": "SceneLockKeys.qml" },
        { "name": "06-peripherals", "scene": "ScenePeripherals.qml" },
        { "name": "07-now-playing", "scene": "SceneNowPlaying.qml" },
        { "name": "08-personalize", "scene": "ScenePersonalize.qml" }
    ]

    property int languageIndex: -1
    property int index: -1
    property var live: null

    function next() {
        if(director.live) {
            director.live.destroy()
            director.live = null
        }
        director.index++
        while(director.index < director.posters.length && !showcase.wanted(director.posters[director.index].name))
            director.index++
        if(director.languageIndex < 0 || director.index >= director.posters.length) {
            director.languageIndex++
            if(director.languageIndex >= showcase.languages.length) {
                showcase.finish()
                return
            }
            showcase.setLanguage(showcase.languages[director.languageIndex])
            director.index = -1
            Qt.callLater(director.next)
            return
        }
        var poster = director.posters[director.index]
        var component = Qt.createComponent(Qt.resolvedUrl(poster.scene))
        if(component.status === Component.Ready)
            director.live = component.createObject(null, { "name": poster.name })
        if(!director.live || !showcase.prepare(director.live)) {
            showcase.fail("could not create poster " + poster.name + ": " + component.errorString())
            Qt.callLater(director.next)
            return
        }
        settle.restart()
    }

    property Timer settle: Timer {
        interval: 300
        onTriggered: {
            showcase.savePoster(director.live, director.posters[director.index].name)
            director.next()
        }
    }

    Component.onCompleted: Qt.callLater(director.next)
}
