/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/



import QtQuick              2.5
import QtQuick.Controls     1.2
import QtQuick.Dialogs      1.2
//import QtQuick.Layouts      1.2

import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.ScreenTools   1.0


SetupPage {
    id:             wifiPage
    pageComponent:  pageComponent

    Component {
        id: pageComponent

        Column {
            id:       settingColumn
            spacing:  _margins
            width:    availableWidth

            WiFiSetupComponentController {
                id:        controller
                factPanel: wifiPage.viewPanel
            }

            QGCPalette { id: palette; colorGroupEnabled: true }

            ExclusiveGroup { id: networksGroup }

            property real _margins:      ScreenTools.defaultFontPixelHeight
            property int  _firstColumn:  ScreenTools.defaultFontPixelWidth * 15
            property int  _secondColumn: ScreenTools.defaultFontPixelWidth * 25
            property string _currentNetwork: ""

            QGCLabel {
                id:             connectedText
                text:           controller.connectionName ? qsTr("Current Wi-Fi network: %1").arg(controller.connectionName) : qsTr("In Access Point Mode")
                font.pointSize: ScreenTools.mediumFontPointSize
                wrapMode:       Text.WordWrap
            }

            QGCButton {
                id:         modeAPButton
                text:       qsTr("Start in AP mode")
                onClicked:  controller.startAPMode()
            }

            QGCLabel {
                text:           qsTr("Saved Networks")
                font.pointSize: ScreenTools.mediumFontPointSize
                wrapMode:       Text.WordWrap
            }

            Rectangle {
                height: 1
                width:  100
                color:  qgcPal.button
            }

            Repeater {
                model: controller.networks
                delegate:
                QGCButton {
                    text:   modelData
                    anchors.leftMargin: ScreenTools.defaultFontPixelWidth * 2
                    exclusiveGroup: networksGroup
                    onClicked: {
                        checked = true
                        settingColumn._currentNetwork = modelData
                    }
                }
            }

            Rectangle {
                height: 1
                width:  100
                color:  qgcPal.button
            }

            Row {
                id:  networksButtonRow
                spacing:    ScreenTools.defaultFontPixelWidth

                QGCButton {
                    width:      ScreenTools.defaultFontPixelWidth * 10
                    text:       qsTr("Connect")
                    enabled:    _currentNetwork !== ""
                    onClicked: {
                        connectDialog.visible = true
                    }
                    MessageDialog {
                        id:         connectDialog
                        visible:    false
                        icon:       StandardIcon.Warning
                        standardButtons: StandardButton.Yes | StandardButton.No
                        title:      qsTr("Connect to Network: %1").arg(settingColumn._currentNetwork)
                        text:       qsTr("Do you want to connect to a selected Wi-Fi network?")
                        onYes: {
                            controller.connectToNetwork(settingColumn._currentNetwork)
                            connectDialog.visible = false
                        }
                        onNo: {
                            connectDialog.visible = false
                        }
                    }
                }

                QGCButton {
                    width:      ScreenTools.defaultFontPixelWidth * 10
                    text:       qsTr("Delete")
                    enabled:    _currentNetwork !== ""
                    onClicked: {
                        deleteDialog.visible = true
                    }
                    MessageDialog {
                        id:         deleteDialog
                        visible:    false
                        icon:       StandardIcon.Warning
                        standardButtons: StandardButton.Yes | StandardButton.No
                        title:      qsTr("Remove Network: %1").arg(settingColumn._currentNetwork)
                        text:       qsTr("Do you want to remove a Wi-Fi network from list?")
                        onYes: {
                            controller.removeNetwork(settingColumn._currentNetwork)
                            deleteDialog.visible = false
                        }
                        onNo: {
                            deleteDialog.visible = false
                        }
                    }
                }
            }

            QGCLabel {
                text:           qsTr("Add a new network")
                font.pointSize: ScreenTools.mediumFontPointSize
                wrapMode:       Text.WordWrap
            }

            Row {
                spacing:    ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:  _firstColumn
                    text:  qsTr("Network name:")
                    anchors.verticalCenter: parent.verticalCenter
                }

                QGCTextField {
                    id:              nameField
                    width:           _secondColumn
                    maximumLength :  32
                }
            }

            Row {
                spacing:    ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:  _firstColumn
                    text:   qsTr("Type:")
                    anchors.verticalCenter: parent.verticalCenter
                }

                QGCComboBox {
                    id:      typeCombo
                    width:   _secondColumn
                    model:   controller.protocolTypes
                }
            }

            Row {
                spacing:    ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:  _firstColumn
                    text:   qsTr("Password:")
                    anchors.verticalCenter: parent.verticalCenter
                }

                QGCTextField {
                    id:              passwordField
                    width:           _secondColumn
                    maximumLength:   64
                    echoMode:        TextInput.Password
                    enabled:         typeCombo.currentIndex != 0
                }
            }

            QGCCheckBox {
                id:        passwordVisibleCheckBox
                text:      qsTr("show password")
                checked:   passwordField.echoMode == TextInput.Normal

                onClicked: passwordField.echoMode = (checked ? TextInput.Normal : TextInput.Password)
            }

            QGCButton {
                text:       "Save"
                onClicked:  {
                    saveDialog.visible = true
                }
                MessageDialog {
                    id:         saveDialog
                    visible:    false
                    icon:       StandardIcon.Warning
                    standardButtons: StandardButton.Yes | StandardButton.No
                    title:      qsTr("Save new Network")
                    text:       qsTr("Do you want add a new Wi-Fi network to list?")
                    onYes: {
                        controller.addNetwork(nameField.text,typeCombo.currentIndex,passwordField.text)
                        saveDialog.visible = false
                    }
                    onNo: {
                        saveDialog.visible = false
                    }
                }
            }
        }
    }
}
