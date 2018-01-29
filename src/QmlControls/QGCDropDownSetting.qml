import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controllers   1.0
import QGroundControl.ScreenTools   1.0

Item {
    width:  parent.width
    height: mouseArea.height

    id:    dropDownSetting
    signal stateChanged

    property alias columns:      grid.columns
    property alias rows:         grid.rows
    property alias title:        settingName.text
    property alias initialState: arrowImage.initialState
    property alias currentState: arrowImage.state

    readonly property alias closedState: arrowImage.closedState
    readonly property alias openedState: arrowImage.openedState

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    MouseArea {
        id:                  mouseArea
        anchors.left:        parent.left
        anchors.right:       parent.right
        width:               parent.width - ScreenTools.defaultFontPixelWidth * 2
        height:              grid.height + ScreenTools.defaultFontPixelWidth * 2

        GridLayout {
            id:                       grid
            anchors.verticalCenter:   parent.verticalCenter
            anchors.left:             parent.left
            anchors.right:            parent.right
            rowSpacing:               ScreenTools.defaultFontPixelWidth
            columns:                  2
            rows:                     1


            QGCLabel {
                id:                 settingName
                anchors.left:       parent.left
                font.family:        ScreenTools.demiboldFontFamily
            }

            QGCColoredImage {
                id:            arrowImage
                width:         ScreenTools.defaultFontPixelHeight / 2
                height:        width
                source:        "/qmlimages/arrow-down.png"
                anchors.right: parent.right

                readonly property string closedState:  "closed"
                readonly property string openedState:  "opened"
                property string          initialState: closedState

                function toggleState() {
                    state = state == closedState ?
                            openedState : closedState;
                }

                states: [
                    State {
                        name: "closed";
                        PropertyChanges { target: arrowImage; rotation: 0   }
                    },
                    State {
                        name: "opened";
                        PropertyChanges { target: arrowImage; rotation: 180 }
                    }
                ]

                transitions: Transition {
                    RotationAnimation { duration: 200; direction: RotationAnimation.Clockwise }
                }

                Component.onCompleted: {
                    arrowImage.state = arrowImage.initialState
                }
            }
        }

        Rectangle {
            color:          arrowImage.state == openedState ?
                                qgcPal.primaryButton : qgcPal.windowShade
            height:         2
            width:          parent.width
            anchors.bottom: parent.bottom
        }


        onClicked: {
            arrowImage.toggleState()
            dropDownSetting.stateChanged()
        }
    }
}
