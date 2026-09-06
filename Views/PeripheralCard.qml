import QtQuick
import MediaFlyouts

// One peripheral card. Shows icon, name, event, and battery ring.
Item {
    id: card

    required property var alert

    readonly property bool hasBattery: alert.percent >= 0
    readonly property string headline: {
        switch(alert.event) {
        case "connected":       return "Connected"
        case "disconnected":    return "Disconnected"
        case "lowBattery":      return "Battery low"
        case "veryLowBattery":  return "Battery very low"
        case "chargingStarted": return "Charging"
        case "chargingStopped": return "Unplugged"
        }
        return ""
    }

    width: 176
    height: 76

    Row {
        anchors.centerIn: parent
        spacing: 10

        BatteryRing {
            anchors.verticalCenter: parent.verticalCenter
            width: 40
            height: 40
            thickness: 4
            // Device with no reading still shows empty ring outline.
            percent: card.hasBattery ? card.alert.percent : 0
            visible: true

            Text {
                anchors.centerIn: parent
                text: DeviceIcons.glyph(card.alert.name, card.alert.connectionKind, card.alert.category)
                font.family: DeviceIcons.family(card.alert.name, card.alert.connectionKind, card.alert.category)
                font.pixelSize: 16
                color: theme.accent
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Text {
                width: 118
                text: card.alert.name
                color: theme.textPrimary
                font.pixelSize: 12
                font.bold: true
                elide: Text.ElideRight
            }
            Text {
                width: 118
                text: {
                    var link = DeviceIcons.transport(card.alert.connectionKind)
                    return link ? card.headline + "  \u00B7  " + link : card.headline
                }
                color: theme.textMuted
                font.pixelSize: 10
                elide: Text.ElideRight
            }
            Text {
                visible: card.hasBattery
                width: 118
                text: card.alert.bucket + "  \u00B7  " + card.alert.percent + "%"
                color: theme.textPrimary
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }
    }
}
