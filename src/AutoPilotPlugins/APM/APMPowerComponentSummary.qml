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

import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0

FactPanel {
    id:             panel
    anchors.fill:   parent
    color:          qgcPal.windowShadeDark

    QGCPalette { id: qgcPal; colorGroupEnabled: enabled }
    FactPanelController { id: controller; factPanel: panel }

    property Fact battCapacity: controller.getParameterFact(-1, "BATT_CAPACITY")
    property Fact battMonitor:  controller.getParameterFact(-1, "BATT_MONITOR")

    property var _getCapacity: function( battnum ) {
        var parameterName = "BATT%1_CAPACITY".arg(battnum === 1? "" : battnum )
        var cap = controller.getParameterFact(-1, parameterName)
        return cap.valueString + "" + cap.units
    }

    property var _getMonitor: function(battnum) {
        var parameterName = "BATT%1_MONITOR".arg(battnum === 1 ? "" : battnum)
        return controller.getParameterFact(-1, parameterName).enumStringValue
    }

    Column {
        anchors.fill:       parent

        Repeater {
            model: 2

            Column {
                property int moduleNum:  index + 1
                anchors.left:            parent.left
                anchors.right:           parent.right

                QGCLabel {
                    text:                      "Power module " + (moduleNum)
                    color:                     "#909090"
                    //anchors.horizontalCenter:  parent.horizontalCenter
                }

                VehicleSummaryRow {
                    labelText: qsTr("monitor: "); valueText: _getMonitor(moduleNum)
                }

                VehicleSummaryRow {
                    labelText: qsTr("capacity: ");  valueText: _getCapacity(moduleNum)
                }

                VehicleSummaryRow { labelText: ""; valueText: "" } // as space
            }
        }
    } //Column
}
