/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts  1.2

import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Palette               1.0

//-------------------------------------------------------------------------
//-- Battery Indicator
Item {
    anchors.top:    parent.top
    anchors.bottom: parent.bottom
    width:          batteryIndicatorRow.width

    property var _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle

    function _getBattery(batteryId) {
        if (_activeVehicle) {
            return batteryId === 1 ? _activeVehicle.battery1 : _activeVehicle.battery2;
        }
    }

    function getBatteryColor(batteryId) {
        if(_activeVehicle) {
            var percentRemaining = _getBattery(batteryId).percentRemaining.value;

            if (percentRemaining > 75.0) {
                return qgcPal.text
            } else if (percentRemaining > 50.0) {
                return qgcPal.colorOrange
            } else {
                return qgcPal.colorRed
            }
        }
        return qgcPal.colorGrey
    }

    function getBatteryPercentageText(batteryId) {
        if (_activeVehicle) {
            var battery = _getBattery(batteryId);

            if (battery.percentRemaining.value > 0.01) {
               return battery.percentRemaining.value > 98.9 ?
                   "100%" : battery.percentRemaining.valueString + battery.percentRemaining.units;
            } else if (battery.voltage.value >= 0) {
                return battery.voltage.valueString + battery.voltage.units
            }
        }
        return "N/A"
    }

    function getBatteryVoltage(batteryId) {
        if (_activeVehicle) {
            var voltageFact = _getBattery(batteryId).voltage;
            if (voltageFact.value != -1.0) {
                return voltageFact.valueString + " " + voltageFact.units
            }
        }

        return "N/A"
    }

    function getMahConsumed(batteryId) {
        if (_activeVehicle) {
            var mahConsumedFact = _getBattery(batteryId).mahConsumed
            if (mahConsumedFact.value !== -1) {
                return mahConsumedFact.valueString + " " + mahConsumedFact.units
            }
        }
        return "N/A"
    }

    Component {
        id: batteryInfo

        Rectangle {
            width:  battCol.width   + ScreenTools.defaultFontPixelWidth  * 3
            height: battCol.height  + ScreenTools.defaultFontPixelHeight * 2
            radius: ScreenTools.defaultFontPixelHeight * 0.5
            color:  qgcPal.window
            border.color:   qgcPal.text

            Column {
                id:                 battCol
                spacing:            ScreenTools.defaultFontPixelHeight * 0.5
                width:              Math.max(batteryStatusColumn.width, batteryStatusHeader.width)
                anchors.margins:    ScreenTools.defaultFontPixelHeight
                anchors.centerIn:   parent

                QGCLabel {
                    id:                        batteryStatusHeader
                    text:                      qsTr("Battery Status")
                    font.family:               ScreenTools.demiboldFontFamily
                    anchors.horizontalCenter:  parent.horizontalCenter
                }

                Column {
                    id:       batteryStatusColumn
                    spacing:  20

                    Repeater {
                        model: _activeVehicle ? [1, 2] : 0

                            GridLayout {
                                anchors.left:      parent.left
                                columns:           2

                                QGCLabel {
                                    Layout.columnSpan:  parent.columns
                                    text:               modelData
                                    color:              "#909090"
                                }

                                QGCLabel { text: qsTr("Voltage:") }
                                QGCLabel { text: getBatteryVoltage(modelData) }

                                QGCLabel { text: qsTr("Accumulated Consumption:") }
                                QGCLabel { text: getMahConsumed(modelData) }
                            }
                    }
                }//Column
            }

            Component.onCompleted: {
                var pos = mapFromItem(toolBar, centerX - (width / 2), toolBar.height)
                x = pos.x
                y = pos.y + ScreenTools.defaultFontPixelHeight
            }
        }
    }

    Row {
        id:             batteryIndicatorRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom

        QGCColoredImage {
            anchors.top:        parent.top
            anchors.bottom:     parent.bottom
            width:              height
            sourceSize.width:   width
            source:             "/qmlimages/Battery.svg"
            fillMode:           Image.PreserveAspectFit
            color:              qgcPal.text
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                // Only two power modules percentage need to show
                model: _activeVehicle ? [1, 2] : 0;

                QGCLabel {
                    text:                   getBatteryPercentageText(modelData)
                    font.pointSize:         ScreenTools.mediumFontPointSize
                    color:                  getBatteryColor(modelData)
                }
            }
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showPopUp(batteryInfo, mapToItem(toolBar, x, y).x + (width / 2))
    }
}
