import QtQuick
import QtQuick.Controls.Basic
import MediaFlyouts

Rectangle {
    id: section
    default property alias contentData: body.data
    property alias text: header.text
    property string glyph: ""
    property string description: ""
    property alias expanded: header.checked

    implicitHeight: header.height + (expanded ? body.implicitHeight + 36 : 0)
    radius: 12
    color: theme.card
    border.width: 1
    border.color: header.visualFocus ? theme.accent : theme.border

    property Button _header: Button {
        id: header
        objectName: "sectionHeader"
        parent: section
        width: parent.width
        checkable: true
        checked: true
        hoverEnabled: true
        focusPolicy: Qt.StrongFocus
        padding: 16
        leftPadding: 18
        rightPadding: 18
        implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding
        font.pixelSize: 14
        font.weight: Font.DemiBold

        background: Rectangle {
            radius: section.radius
            color: header.down || header.hovered ? theme.cardHover : "transparent"
            Behavior on color { ColorAnimation { duration: Anim.hoverMs } }
        }

        contentItem: Item {
            implicitHeight: Math.max(36, labels.implicitHeight)

            Rectangle {
                id: iconTile
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 36
                height: 36
                radius: 10
                color: theme.accentBadge

                IconLabel {
                    anchors.centerIn: parent
                    glyph: section.glyph
                    size: 20
                    color: theme.accent
                }
            }

            Column {
                id: labels
                anchors.left: iconTile.right
                anchors.leftMargin: 14
                anchors.right: chevron.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Text {
                    width: parent.width
                    text: header.text
                    color: theme.textPrimary
                    font: header.font
                    elide: Text.ElideRight
                }
                Text {
                    visible: section.description.length > 0
                    width: parent.width
                    text: section.description
                    color: theme.textSecondary
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Item {
                id: chevron
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 16
                height: 16
                rotation: header.checked ? 0 : -90
                Behavior on rotation { NumberAnimation { duration: Anim.moveMs; easing.type: Easing.OutCubic } }

                Rectangle {
                    x: 2
                    y: 6
                    width: 7
                    height: 1.5
                    radius: 0.75
                    rotation: 45
                    color: theme.textSecondary
                }
                Rectangle {
                    x: 7
                    y: 6
                    width: 7
                    height: 1.5
                    radius: 0.75
                    rotation: -45
                    color: theme.textSecondary
                }
            }
        }
    }

    property Rectangle _divider: Rectangle {
        parent: section
        x: 18
        y: header.height
        width: parent.width - 36
        height: 1
        color: theme.border
        visible: section.expanded
    }

    property Column _body: Column {
        id: body
        parent: section
        x: 18
        y: header.height + 18
        width: parent.width - 36
        spacing: 12
        visible: section.expanded
    }
}
