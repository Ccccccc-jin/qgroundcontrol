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

    property var corePlugin: QGroundControl.corePlugin

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

    function disabledVisible(button) {
        button.visible = true
        button.enabled = false
    }

    function enabledVisible(button) {
        button.visible = true
        button.enabled = true
    }

    function notVisible(button) {
        button.visible = false
    }

    function enableVehicleSetupButtons() {
        corePlugin.vehicleSetupDisabled = false
    }

    function disableVehicleSetup() {
        corePlugin.vehicleSetupDisabled = true
    }


    FirmwareUpgradeController {
        id: fwUpgradeController

        /* States functions. These function was defined for switching between window states.
           This is not best solution. Perfectly, this code should be re-wrote with qml StateMachine. */

        function initialState() {
            disabledVisible(connectButton)
            notVisible(cancelButton)

            progressBar.changeProgressBarValue(0)
        }

        function deviceUnpluggedState() {
            initialState()
            var msg = "Device unplugged."
            statusTextArea.appendInfoMessage(msg)
        }

        function askForPriviledgesState() {
            initialState()
            disableVehicleSetup()
        }

        function deviceAvailableState() {
            enabledVisible(connectButton)
            notVisible(cancelButton)

            var msg = "Emlid edge device found. Press \"Connect\" to continue."
            statusTextArea.appendInfoMessage(msg)
        }

        function flashingFinishedState() {
            if (fwUpgradeController.deviceAvailable()) {
                enabledVisible(connectButton)
            } else {
                disabledVisible(connectButton)
            }

            notVisible(cancelButton)
            enableVehicleSetupButtons()
        }

        function flashingCancelledState() {
            var msg = "Firmware Upgrading cancelled."
            statusTextArea.appendInfoMessage(msg)
            enableVehicleSetupButtons()
        }

        function flashingStartedState() {
            notVisible(connectButton)
            enabledVisible(cancelButton)
        }

        function flashingSuccessfullyFinishedState() {
            notVisible(connectButton)
            disabledVisible(cancelButton)

            var msg = "Firmware upgrading finished. Please unplug your device."
            statusTextArea.appendInfoMessage(msg)
            enableVehicleSetupButtons()
        }

        function flashingFailedState() {
            notVisible(connectButton)
            disabledVisible(cancelButton)

            var msg = "An error ocurred while flashing device." +
                      "Make sure that you did not disconnect your device."
            statusTextArea.appendErrorMessage(msg)
            enableVehicleSetupButtons()
        }

        onDevicePlugged: {
            deviceAvailableState()
        }

        onDeviceUnplugged: {
            deviceUnpluggedState()
        }

        onReady: {
            var dialogTitle = "Firmware settings"

            showDialog(firmwareUpgraderDialogComponent,
                       dialogTitle, qgcView.showDialogDefaultWidth,
                       StandardButton.Ok | StandardButton.Cancel)
        }

        onFinished: {
            flashingFinishedState()
        }

        onFlashingFinished: {
            status ? flashingSuccessfullyFinishedState()
                   : flashingFailedState()
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

        onWarnMsgReceived: {
            statusTextArea.appendWarnMessage(message);
        }

        Component.onCompleted: {
            initialState()
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
            width:               parent.width - Math.max(cancelButton.width, connectButton.width) - 2 * ScreenTools.defaultFontPixelWidth

            function changeProgressBarValue(value) {
                progressBar.value = value
            }
        }

        QGCButton {
            id:                  connectButton
            text:                "Connect"

            enabled:             false
            height:              progressBar.height

            anchors.leftMargin:  ScreenTools.defaultFontPixelWidth
            anchors.rightMargin: ScreenTools.defaultFontPixelWidth
            anchors.topMargin:   ScreenTools.defaultFontPixelHeight
            anchors.right:       parent.right
            anchors.top:         titleLabel.bottom


            onClicked: {
                fwUpgradeController.start()
                fwUpgradeController.askForPriviledgesState()
            }
        }

        QGCButton {
            id:                  cancelButton
            text:                "Cancel"

            visible:             false
            height:              progressBar.height

            anchors.leftMargin:  ScreenTools.defaultFontPixelWidth
            anchors.rightMargin: ScreenTools.defaultFontPixelWidth
            anchors.topMargin:   ScreenTools.defaultFontPixelHeight
            anchors.right:       parent.right
            anchors.top:         titleLabel.bottom

            onClicked: {
                fwUpgradeController.cancel()
                fwUpgradeController.flashingCancelledState()
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
                colouredMessage("info", "green", message)
            }

            function appendWarnMessage(message) {
                colouredMessage("warning", "orange", message)
            }

            function appendErrorMessage(message) {
                colouredMessage("error", "red", message)
            }

            function colouredMessage(type, color, message) {
                statusTextArea.append("<font color=\"" + color + "\">" + type + "</font> : " + message + "\n")
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

              function reject() {
                  hideDialog();
                  fwUpgradeController.cancel()
                  fwUpgradeController.initialState()
              }

              function accept() {
                  if (fwUpgradeController.firmwareFilename != "") {
                      hideDialog()
                      fwUpgradeController.flash()
                      fwUpgradeController.flashingStartedState()
                  } else {
                      statusTextArea.appendWarnMessage("Incorrect firmware file.")
                  }
              }

              Column {
                  anchors.fill: parent
                  spacing: ScreenTools.defaultFontPixelHeight

                  QGCLabel {
                      anchors.horizontalCenter: parent.horizontalCenter
                      id:   firmwareVersion
                      text: "Current firmware version: " + fwUpgradeController.firmwareVersion;
                  }

                  Row {
                      spacing: ScreenTools.defaultFontPixelHeight
                      anchors.horizontalCenter: parent.horizontalCenter

                      QGCLabel {
                          id: checkBoxTitle
                          text: "Enable checksum"
                      }

                      QGCCheckBox {
                          id:      checksumCheckbox
                          checked: true

                          onClicked: {
                              fwUpgradeController.checksumEnabled = checksumCheckbox.checked;
                          }

                      }

                  }

                  QGCButton {
                      id: browseButton
                      anchors.horizontalCenter: parent.horizontalCenter
                      text: "Browse image file"

                      onClicked: {
                          fwUpgradeController.askForFirmwareFile()
                      }
                  }

              }
        }

    }
} // QGCView
