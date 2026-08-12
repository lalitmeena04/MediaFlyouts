import QtQuick
import QtQuick.Window
import MediaFlyouts

// A small always-on-top media widget docked on the primary taskbar near the tray.
Window {
    id: root
    width: 220
    height: 40
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus

    // shown only when enabled and something is playing
    visible: settings.taskbarWidgetEnabled && nowPlaying.hasMedia

    signal activated()

    // horizontal gap kept clear for the system tray / clock on the right of the taskbar
    readonly property int trayPadding: 200

    function updatePosition() {
        var tb = taskbar.taskbarGeometry()
        var tray = taskbar.trayGeometry()
        if(tb.width > 0) {
            // anchor just left of the real system-tray boundary so we never overlap it,
            // plus a user offset to clear third-party taskbar widgets (speed meters, etc.)
            var rightEdge = (tray.width > 0) ? tray.x : (tb.x + tb.width - trayPadding)
            root.x = rightEdge - width - 8 - settings.taskbarWidgetOffset
            root.y = tb.y + Math.max(0, (tb.height - height) / 2)
        } else {
            // fallback when the taskbar can't be located
            var taskbarHeight = Screen.height - Screen.desktopAvailableHeight
            root.x = Screen.virtualX + Screen.width - width - trayPadding - settings.taskbarWidgetOffset
            root.y = (taskbarHeight > 0)
                ? Screen.virtualY + Screen.desktopAvailableHeight + Math.max(0, (taskbarHeight - height) / 2)
                : Screen.virtualY + Screen.height - height - 4
        }
    }

    Component.onCompleted: updatePosition()
    onVisibleChanged: if(visible) updatePosition()

    // reposition immediately when the offset (or other settings) change
    Connections {
        target: settings
        function onChanged() { root.updatePosition() }
    }

    // re-evaluate placement periodically to follow resolution / taskbar changes
    Timer {
        interval: 2000
        running: root.visible
        repeat: true
        onTriggered: root.updatePosition()
    }

    Rectangle {
        id: card
        anchors.fill: parent
        radius: 6
        color: hover.hovered ? "#2A2A2A" : "transparent"

        HoverHandler {
            id: hover
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.activated()
        }

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Rectangle {
                width: 30
                height: 30
                radius: 5
                color: "#333333"
                clip: true
                anchors.verticalCenter: parent.verticalCenter

                Image {
                    anchors.fill: parent
                    source: nowPlaying.artSource
                    visible: nowPlaying.hasArt
                    fillMode: Image.PreserveAspectCrop
                    cache: false
                    asynchronous: true
                }

                Text {
                    anchors.centerIn: parent
                    visible: !nowPlaying.hasArt
                    text: Fluent.musicNote
                    font.family: Fluent.fontFamily
                    color: "#4CC2FF"
                    font.pixelSize: 18
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0
                width: controlsRow.visible ? 78 : 132

                Text {
                    width: parent.width
                    text: nowPlaying.title
                    color: "white"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: nowPlaying.artist
                    color: "#AAAAAA"
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }

            Row {
                id: controlsRow
                visible: settings.taskbarWidgetControlsEnabled
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                IconLabel {
                    glyph: Fluent.previous
                    size: 14
                    color: prevArea.containsMouse ? "white" : "#C8C8C8"
                    MouseArea {
                        id: prevArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: nowPlaying.previous()
                    }
                }
                IconLabel {
                    glyph: nowPlaying.isPlaying ? Fluent.pause : Fluent.play
                    size: 16
                    color: "white"
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: nowPlaying.playPause()
                    }
                }
                IconLabel {
                    glyph: Fluent.next
                    size: 14
                    color: nextArea.containsMouse ? "white" : "#C8C8C8"
                    MouseArea {
                        id: nextArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: nowPlaying.next()
                    }
                }
            }
        }
    }
}
