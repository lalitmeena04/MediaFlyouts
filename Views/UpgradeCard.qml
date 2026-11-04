import QtQuick
import QtQuick.Controls.Basic

// Home page card. The whole card starts the Pro purchase; the link at the
// bottom re-reads the licence for buyers the Store has not recognised yet.
Rectangle {
    id: card
    property var ownerWindow: null
    property string statusText: ""
    property bool checkingLicense: false

    readonly property bool clickable: entitlement.canPurchase
    readonly property bool canCheckLicense: entitlement.storeAvailable && !entitlement.purchasing
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

    function checkLicense() {
        if(!card.canCheckLicense || card.checkingLicense)
            return
        card.statusText = ""
        card.checkingLicense = true
        entitlement.refresh()
    }

    Connections {
        target: entitlement
        function onPurchaseFinished(ok, statusId, storeId) {
            if(storeId === entitlement.proAddOnStoreId)
                card.statusText = card.statusMessage(statusId)
        }
        function onChanged() {
            if(!card.checkingLicense || entitlement.refreshing)
                return
            card.checkingLicense = false
            // A Pro answer hides the card; only the miss needs words.
            if(!entitlement.isPro)
                card.statusText = qsTr("No Pro license found for this Microsoft account. Sign in to the Microsoft Store with the account that bought the app, then try again.")
        }
    }

    Row {
        id: content
        // Above the purchase MouseArea, so the license link gets its own clicks.
        z: 1
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
            Text {
                id: licenseLink
                visible: card.canCheckLicense
                width: parent.width
                text: card.checkingLicense ? qsTr("Checking with the Microsoft Store…")
                                           : qsTr("Already bought MediaFlyouts? Check your license")
                color: card.checkingLicense ? theme.textTertiary : theme.textSecondary
                font.pixelSize: 12
                font.underline: !card.checkingLicense && linkMouse.containsMouse
                wrapMode: Text.WordWrap
                topPadding: 2

                MouseArea {
                    id: linkMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !card.checkingLicense
                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: card.checkLicense()
                }
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
