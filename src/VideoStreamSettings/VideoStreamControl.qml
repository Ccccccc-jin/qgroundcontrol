
/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick                          2.3
import QtQuick.Controls                 1.2
import QtQuick.Dialogs                  1.2
import QtGraphicalEffects               1.0

import QGroundControl                   1.0
import QGroundControl.ScreenTools       1.0
import QGroundControl.Controls          1.0
import QGroundControl.Palette           1.0
import QGroundControl.Vehicle           1.0
import QGroundControl.Controllers       1.0
import QGroundControl.FactSystem        1.0
import QGroundControl.FactControls      1.0


Rectangle {
    id:             mainRect
    height:         videoStreamSettingsCol.height * 1.5
    width:          videoStreamSettingsCol.width  * 1.5
    radius:         ScreenTools.defaultFontPixelWidth * 0.5
    color:          qgcPal.globalTheme === QGCPalette.Light ? Qt.rgba(1,1,1,0.95) : Qt.rgba(0,0,0,0.75)
    border.width:   1
    border.color:   qgcPal.globalTheme === QGCPalette.Light ? Qt.rgba(0,0,0,0.35) : Qt.rgba(1,1,1,0.35)

    property var  _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property var  _controller: _activeVehicle ? _activeVehicle.videoStreamManager : null
    property bool _communicationLost: _activeVehicle ? _activeVehicle.connectionLost : false

    property real _margins:      ScreenTools.defaultFontPixelWidth
    property int  _firstColumn:  ScreenTools.defaultFontPixelWidth * 10
    property int  _secondColumn: ScreenTools.defaultFontPixelWidth * 15

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    Image {
        anchors.margins:    ScreenTools.defaultFontPixelHeight * 0.5
        anchors.top:        parent.top
        anchors.right:      parent.right
        width:              ScreenTools.isMobile ? ScreenTools.defaultFontPixelHeight * 1.5 : ScreenTools.defaultFontPixelHeight
        height:             width
        sourceSize.height:  width
        source:             "/res/XDelete.svg"
        fillMode:           Image.PreserveAspectFit
        MouseArea {
            anchors.fill:       parent
            anchors.margins:    ScreenTools.isMobile ? -ScreenTools.defaultFontPixelHeight : 0
            onClicked: {
                mainRect.visible = false
            }
        }
    }

    QGCLabel {
        id:                 videoStreamSettingsLabel
        text:               qsTr("Video streaming settings")
        font.family:        ScreenTools.demiboldFontFamily
        font.pointSize:     ScreenTools.mediumFontPointSize
        anchors.margins:    ScreenTools.defaultFontPixelHeight * 0.5
        anchors.top:        parent.top
        anchors.left:       parent.left
    }

    QGCFlickable {
        clip:               true
        anchors.top:        videoStreamSettingsLabel.bottom
        anchors.topMargin:  ScreenTools.defaultFontPixelHeight
        anchors.left:       parent.left
        anchors.bottom:     parent.bottom
        width:              videoStreamSettingsCol.width
        contentHeight:      videoStreamSettingsCol.height
        contentWidth:       videoStreamSettingsCol.width

        Column {
            id:                 videoStreamSettingsCol
            spacing:            ScreenTools.defaultFontPixelHeight * 0.5
            anchors.left:       parent.left
            anchors.leftMargin: ScreenTools.defaultFontPixelHeight
            anchors.margins:    ScreenTools.defaultFontPixelHeight

            Row {
                spacing: ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:             _firstColumn
                    text:              qsTr("IP adress:")
                    anchors.baseline:  ipField.baseline
                }

                QGCTextField {
                    id:                ipField
                    text:              _controller.targetIp
                    width:             _secondColumn
                }
            }

            Row {
                spacing: ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:             _firstColumn
                    text:              qsTr("Port:")
                    anchors.baseline:  portField.baseline
                }

                QGCTextField {
                    id:                portField
                    text:              _controller.targetPort
                    width:             _secondColumn
                    validator:         IntValidator { bottom: 0; top: 65535}
                }
            }

            QGCButton {
                text:      "Use IP and Port of GCS"
                onClicked: {
                    portField.text = QGroundControl.settingsManager.videoSettings.udpPort.rawValue
                    ipField.text = _controller.getLocalAddress()
                }
            }

            Row {
                spacing: ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:             _firstColumn
                    anchors.baseline:  resolutionFact.baseline
                    text:              qsTr("Resolution:")
                }

                FactComboBox {
                    id:                resolutionFact
                    fact:              _controller.resolutionFact
                    width:             _secondColumn
                    indexModel:        true
                }
            }

            Row {
                spacing: ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:             _firstColumn
                    anchors.baseline:  bitRateFact.baseline
                    text:              qsTr("Bit rate:")
                }

                FactComboBox {
                    id:                bitRateFact
                    fact:              _controller.bitRateFact
                    width:             _secondColumn
                    indexModel:        false
                }
            }

            Row {
                spacing: ScreenTools.defaultFontPixelWidth

                QGCLabel {
                    width:             _firstColumn
                    anchors.baseline:  rotationFact.baseline
                    text:              qsTr("Rotation:")
                }

                FactComboBox {
                    id:                rotationFact
                    fact:              _controller.rotationFact
                    width:             _secondColumn
                    indexModel:        false
                }
            }

            Row {
                spacing: ScreenTools.defaultFontPixelWidth

                QGCButton {
                    text:       "Save Settings"
                    onClicked:  {
                        _controller.saveSettings(ipField.text, portField.text)
                    }
                }

                QGCButton {
                    text:       "Start Streaming"
                    onClicked:  {
                        _controller.startVideo()
                    }
                }

                QGCButton {
                    text:       "Stop Streaming"
                    onClicked:  {
                        _controller.stopVideo()
                    }
                }

                // FixMe: without it part of previous button will be cut of
                // Seems like because of wrong calucation of videoStreamSettingsCol.width
                Rectangle {
                    width: 10
                    height: 10
                }
            }
        }
    }
}
