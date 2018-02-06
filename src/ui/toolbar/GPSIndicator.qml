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
//-- GPS Indicator
Item {
    id:             satelitte
    width:          (gpsValuesColumn.x + gpsValuesColumn.width + gps2ValuesColumn.width) * 1.1
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    readonly property var __noDataMsg: qsTr("N/A", "No data to display")

    function gpsInfoValueString(gpsParam) {
        return activeVehicle ? gpsParam.valueString : __noDataMsg
    }

    function gpsLockValueString(gps) {
        return activeVehicle ? gps.lock.enumStringValue : __noDataMsg
    }

    function isGpsAvailable(gps) {
        return !isNaN(gps.hdop.value)
    }

    Component {
        id: gpsInfo

        Rectangle {
            width:  gpsCol.width   + ScreenTools.defaultFontPixelWidth  * 3
            height: gpsCol.height  + ScreenTools.defaultFontPixelHeight * 2
            radius: ScreenTools.defaultFontPixelHeight * 0.5
            color:  qgcPal.window
            border.color:   qgcPal.text

            Column {
                id:                 gpsCol
                spacing:            ScreenTools.defaultFontPixelHeight * 0.5
                width:              Math.max(gpsGrid.width, gpsLabel.width)
                anchors.margins:    ScreenTools.defaultFontPixelHeight
                anchors.centerIn:   parent

                QGCLabel {
                    id:                       gpsLabel
                    text:                     (activeVehicle && activeVehicle.gps.count.value >= 0) ? qsTr("GPS Status") : qsTr("GPS Data Unavailable")
                    font.family:              ScreenTools.demiboldFontFamily
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                GridLayout {
                    id:                       gpsGrid
                    visible:                  (activeVehicle && activeVehicle.gps.count.value >= 0)
                    anchors.margins:          ScreenTools.defaultFontPixelHeight
                    columnSpacing:            ScreenTools.defaultFontPixelWidth
                    anchors.horizontalCenter: parent.horizontalCenter
                    columns:                  3

                    QGCLabel { text: ""; horizontalAlignment: Text.AlignHCenter}
                    QGCLabel { Layout.fillWidth: true; text: "GPS1"; color: "#909090"; horizontalAlignment: Text.AlignHCenter }
                    QGCLabel { Layout.fillWidth: true; text: "GPS2"; color: "#909090"; horizontalAlignment: Text.AlignHCenter }

                    QGCLabel { text: qsTr("GPS Count:") }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps.count) }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps2.count) }

                    QGCLabel { text: qsTr("GPS Lock:") }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsLockValueString(activeVehicle.gps) }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsLockValueString(activeVehicle.gps2) }

                    QGCLabel { text: qsTr("HDOP:") }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps.hdop) }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps2.hdop) }

                    QGCLabel { text: qsTr("VDOP:") }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps.vdop) }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps2.vdop) }

                    QGCLabel { text: qsTr("Course Over Ground:") }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps.courseOverGround) }
                    QGCLabel { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: gpsInfoValueString(activeVehicle.gps2.courseOverGround) }
                }
            }

            Component.onCompleted: {
                var pos = mapFromItem(toolBar, centerX - (width / 2), toolBar.height)
                x = pos.x
                y = pos.y + ScreenTools.defaultFontPixelHeight
            }
        }
    }

    QGCColoredImage {
        id:                 gpsIcon
        width:              height
        anchors.top:        parent.top
        anchors.bottom:     parent.bottom
        source:             "/qmlimages/Gps.svg"
        fillMode:           Image.PreserveAspectFit
        sourceSize.height:  height
        opacity:            (activeVehicle && activeVehicle.gps.count.value >= 0) ? 1 : 0.5
        color:              qgcPal.buttonText
    }

    Column {
        id:                     gpsValuesColumn
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin:     ScreenTools.defaultFontPixelWidth / 2
        anchors.left:           gpsIcon.right

        QGCLabel {
            anchors.horizontalCenter:   hdopValue.horizontalCenter
            visible:                    activeVehicle && isGpsAvailable(activeVehicle.gps)
            color:                      qgcPal.buttonText
            text:                       visible ? activeVehicle.gps.count.valueString : ""
        }

        QGCLabel {
            id:         hdopValue
            visible:    activeVehicle && isGpsAvailable(activeVehicle.gps)
            color:      qgcPal.buttonText
            text:       visible ? activeVehicle.gps.hdop.value.toFixed(1) : ""
        }
    }

    Column {
        id:                     gps2ValuesColumn
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin:     ScreenTools.defaultFontPixelWidth
        anchors.left:           gpsValuesColumn.right

        QGCLabel {
            anchors.horizontalCenter:   gps2hdopValue.horizontalCenter
            visible:                    activeVehicle && isGpsAvailable(activeVehicle.gps2)
            color:                      qgcPal.buttonText
            text:                       visible && gpsInfoValueString(activeVehicle.gps2.count)
        }

        QGCLabel {
            id:         gps2hdopValue
            visible:    activeVehicle && isGpsAvailable(activeVehicle.gps2)
            color:      qgcPal.buttonText
            text:       visible ? activeVehicle.gps2.hdop.value.toFixed(1) : ""
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked: {
            var centerX = mapToItem(toolBar, x, y).x + (width / 2)
            mainWindow.showPopUp(gpsInfo, centerX)
        }
    }
}
