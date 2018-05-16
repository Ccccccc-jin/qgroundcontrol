/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick 2.3
import QtQuick.Controls 1.2

import QGroundControl               1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controllers   1.0

FactPanel {
    id:             panel
    anchors.fill:   parent
    color:          qgcPal.windowShadeDark

    FactPanelController {
        id: controller;
        factPanel: panel

        property WifiManager wifiManager: vehicle.wifiManager
    }

    QGCPalette { id: palette; colorGroupEnabled: true }

    Column {
        anchors.fill:       parent

        VehicleSummaryRow {
            labelText:  qsTr("Mode:")
            valueText:  controller.wifiManager.wifiStateAsString(controller.wifiManager.wifiState)
        }

        VehicleSummaryRow {
            labelText:  qsTr("Network SSID:")
            valueText: controller.wifiManager.activeNetworkSsid
            visible:   controller.wifiManager.activeNetworkSsid !== ""
        }
    }
}
