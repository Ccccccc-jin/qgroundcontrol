/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controllers   1.0
import QGroundControl.ScreenTools   1.0


QGCView {
    id:         qgcView
    viewPanel:  panel

    readonly property string title:             "FIRMWARE"
    readonly property string welcomeText:       qsTr("%1 can upgrade the firmware on Emlid Edge device.\n").arg(QGroundControl.appName) +
                                                qsTr("Plug in your device and wait for auto detection.")

    QGCPalette { id: qgcPal; colorGroupEnabled: true }

    Component {
        id: firmwareWarningDialog

        QGCViewMessage {
            message: firmwareWarningMessage

            function accept() {
                hideDialog()
                controller.doFirmwareUpgrade()
            }
        }
    }

    FirmwareUpgradeController {
        id: fwUpgradeController

        onDeviceFound: {
            var dialogTitle = "Firmware settings"
            showDialog(firmwareUpgraderDialogComponent,
                       dialogTitle, qgcView.showDialogDefaultWidth,
                       StandardButton.Ok | StandardButton.Cancel)
        }

        onFlasherProgressChanged: {
            progressBar.changeProgressBarValue(progress);
        }

        onInfoMsgReceived: {
            statusTextArea.appendInfoMessage(message);
        }

        onErrorMsgReceived: {
            statusTextArea.appendErrorMessage(message);
        }

        Component.onCompleted: {
            fwUpgradeController.searchDevice()
        }
    }


    QGCViewPanel {
        id:             panel
        anchors.fill:   parent

        QGCLabel {
            id:             titleLabel
            text:           title
            font.pointSize: ScreenTools.mediumFontPointSize
        }

        ProgressBar {
            id:                  progressBar
            anchors.topMargin:   ScreenTools.defaultFontPixelHeight
            anchors.top:         titleLabel.bottom
            minimumValue:        0
            maximumValue:        100
            width:               parent.width - cancelButton.width - 2 * ScreenTools.defaultFontPixelWidth

            function changeProgressBarValue(value) {
                progressBar.value = value
            }
        }

        QGCButton {
            id:                  cancelButton
            text:                "Cancel"

            enabled:             false
            height:              progressBar.height

            anchors.leftMargin:  ScreenTools.defaultFontPixelWidth
            anchors.rightMargin: ScreenTools.defaultFontPixelWidth
            anchors.topMargin:   ScreenTools.defaultFontPixelHeight
            anchors.right:       parent.right
            anchors.top:         titleLabel.bottom

            onClicked: {
                fwUpgradeController.cancelFlashing();
            }
        }

        TextArea {
            id:                 statusTextArea
            anchors.topMargin:  ScreenTools.defaultFontPixelHeight
            anchors.top:        cancelButton.bottom
            anchors.bottom:     parent.bottom
            width:              parent.width
            readOnly:           true
            frameVisible:       false
            font.pointSize:     ScreenTools.defaultFontPointSize
            textFormat:         TextEdit.RichText
            text:               welcomeText

            function appendInfoMessage(message) {
                colouredMessage("info", "green", message);
            }

            function appendWarnMessage(message) {
                colouredMessage("warning", "orange", message);
            }

            function appendErrorMessage(message) {
                colouredMessage("error", "red", message);
            }

            function colouredMessage(type, color, message) {
                statusTextArea.append("<font color=\"" + color + "\">" + type + "</font> : " + message + "\n");
            }

            style: TextAreaStyle {
                textColor:          qgcPal.text
                backgroundColor:    qgcPal.windowShade
            }
        }
    }


    Component {
        id: firmwareUpgraderDialogComponent

        QGCViewDialog {
              id: firmwareUpgraderDialog
              anchors.fill: parent

              Column {
                  QGCLabel {
                      id:   header
                      text: "Found Emlid Edge device"
                  }

                  QGCButton {
                      text: "Browse image file"

                      onClicked: {
                          fwUpgradeController.askForFirmwareFile();
                          hideDialog();
                          cancelButton.enabled = true;
                      }
                  }
              }
        }

    }
} // QGCView
