/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs  1.2
import QtQuick.Controls.Styles 1.4

import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.Palette       1.0
import QGroundControl.ScreenTools   1.0

SetupPage {
    id:             vehiclePage
    pageComponent:  vehiclePageComponent

    VehicleSelectorController {
        id:        controller
        factPanel: vehiclePage.viewPanel
    }

    Component {
        id: applyRestartDialogComponent

        QGCViewDialog {
            id: applyRestartDialog

            function accept() {
                controller.changeVehicle()
                applyRestartDialog.hideDialog()
            }

            QGCLabel {
                anchors.fill:   parent
                wrapMode:       Text.WordWrap
                text:           qsTr("Clicking “Apply” will save the changes you have made to your airframe configuration. ") +
                                qsTr("Your vehicle will also be restarted in order to complete the process.")
            }
        }
    }

    Component {
        id: vehiclePageComponent

        Column {
            id:      mainColumn
            spacing: _margins
            width:   availableWidth

            QGCPalette { id: palette; colorGroupEnabled: true }

            property real _margins:  ScreenTools.defaultFontPixelHeight
            property real _minW:     ScreenTools.defaultFontPixelWidth * 30
            property real _boxWidth: _minW
            property real _boxSpace: ScreenTools.defaultFontPixelWidth

            Item {
                id:             helpApplyRow
                anchors.left:   parent.left
                anchors.right:  parent.right
                height:         Math.max(helpText.contentHeight, applyButton.height)

                QGCLabel {
                    id:                     helpText
                    anchors.rightMargin:    _margins
                    anchors.left:           parent.left
                    anchors.right:          applyButton.right
                    text:                   qsTr("Please select your vehicle for Linux board.")
                    font.pointSize:         ScreenTools.mediumFontPointSize
                    wrapMode:               Text.WordWrap
                }

                QGCButton {
                    id:             applyButton
                    anchors.right:  parent.right
                    text:           qsTr("Apply and Restart")

                    onClicked:      showDialog(applyRestartDialogComponent, qsTr("Apply and Restart"), qgcView.showDialogDefaultWidth, StandardButton.Apply | StandardButton.Cancel)
                }
            }

            Flow {
                id:         flowView
                width:      parent.width
                spacing:    _boxSpace

                ExclusiveGroup {
                    id: vehicleTypeExclusive
                }

                Repeater {
                    model: controller.vehicleTypes

                    // Outer summary item rectangle
                    Rectangle {
                        width:  _boxWidth
                        height: ScreenTools.defaultFontPixelHeight * 14
                        color:  qgcPal.window

                        readonly property real titleHeight: ScreenTools.defaultFontPixelHeight * 1.75
                        readonly property real innerMargin: ScreenTools.defaultFontPixelWidth

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {
                                applyButton.primary = true
                                vehicleCheckBox.checked = true
                            }
                        }

                        QGCLabel {
                            id:     title
                            text:   modelData.name
                        }

                        Rectangle {
                            anchors.topMargin:  ScreenTools.defaultFontPixelHeight / 2
                            anchors.top:        title.bottom
                            anchors.bottom:     parent.bottom
                            anchors.left:       parent.left
                            anchors.right:      parent.right
                            color:              vehicleCheckBox.checked ? qgcPal.buttonHighlight : qgcPal.windowShade

                            Image {
                                id:                 image
                                anchors.margins:    innerMargin
                                anchors.top:        parent.top
                                anchors.bottom:     parent.bottom
                                anchors.left:       parent.left
                                anchors.right:      parent.right
                                fillMode:           Image.PreserveAspectFit
                                smooth:             true
                                mipmap:             true
                                source:             modelData.imgSource
                            }

                            QGCCheckBox {
                                // Although this item is invisible we still use it to manage state
                                id:             vehicleCheckBox
                                checked:        modelData.vehicleID == controller.currentVehicleType
                                exclusiveGroup: vehicleTypeExclusive
                                visible:        false

                                onCheckedChanged: {
                                    if (checked) {
                                        controller.currentVehicleType = controller.vehicleTypes[index].vehicleID
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
