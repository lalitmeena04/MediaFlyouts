import QtQuick
import QtQuick.Controls.Basic

// Home page card. The whole card starts the Pro purchase.
Rectangle {
    id: card
    property var ownerWindow: null
    property string statusText: ""

    readonly property bool clickable: entitlement.canPurchase
    visible: entitlement.gatingEnabled && !entitlement.isPro
    width: parent ? parent.width : 400
    height: visible ? content.implicitHeight + 36 : 0
    radius: 12
    color: card.clickable && ma.containsMouse ? theme.cardHover : theme.card
    border.width: 1
    border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.45)

    readonly property string headline: entitlement.isTrial
        ? qsTr("Pro trial: %1 days left").arg(entitlement.trialDaysLeft)
        : qsTr("Your Pro trial has ended")
    readonly property string actionText: entitlement.purchasing
        ? qsTr("Opening the Microsoft Store…")
        : (card.clickable
            ? (entitlement.price.length > 0 ? qsTr("Get Pro for %1").arg(entitlement.price) : qsTr("Get Pro"))
            : qsTr("Pro is sold through the Microsoft Store version of MediaFlyouts."))

    function statusMessage(statusId) {
        switch(statusId) {
        case "succeeded": return qsTr("Thank you! Pro is unlocked.")
        case "alreadyPurchased": return qsTr("You already own Pro. Restoring your purchase…")
        case "notPurchased": return ""
        case "networkError": return qsTr("No connection to the Microsoft Store. Check your network and try again.")
        case "unavailable":
        case "notConfigured": return qsTr("Purchases are not available in this build.")
        case "busy": return ""
        }
        return qsTr("The Microsoft Store could not complete the purchase. Try again later.")
    }

    Connections {
        target: entitlement
        function onPurchaseFinished(ok, statusId, storeId) {
            if(storeId === entitlement.proAddOnStoreId)
                card.statusText = card.statusMessage(statusId)
        }
    }

    Row {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 18
        spacing: 16

        Rectangle {
            width: 48
            height: 48
            radius: 24
            color: theme.accentBadge
            anchors.verticalCenter: parent.verticalCenter
            IconLabel {
                anchors.centerIn: parent
                glyph: Fluent.flash
                size: 24
                color: theme.accent
            }
        }

        Column {
            width: parent.width - 48 - 16
            spacing: 6
            anchors.verticalCenter: parent.verticalCenter

            Row {
                spacing: 8
                Text {
                    text: card.headline
                    color: theme.textPrimary
                    font.pixelSize: 16
                    font.bold: true
                }
                ProBadge { anchors.verticalCenter: parent.verticalCenter }
            }
            Text {
                width: parent.width
                text: qsTr("Pro unlocks the audio visualizer, the per-app mixer, peripheral battery, Now Playing styles and media widget options.")
                color: theme.textSecondary
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            Text {
                width: parent.width
                text: card.actionText
                color: card.clickable ? theme.accent : theme.textTertiary
                font.pixelSize: 13
                font.bold: card.clickable
                wrapMode: Text.WordWrap
            }
            Text {
                visible: card.statusText.length > 0
                width: parent.width
                text: card.statusText
                color: theme.textTertiary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        hoverEnabled: true
        enabled: card.clickable
        cursorShape: card.clickable ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: {
            card.statusText = ""
            entitlement.purchase(card.ownerWindow)
        }
    }
}
