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
import QtQuick.Layouts 1.3
import QtQml.StateMachine 1.0 as DSM

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

    function disallowVehicleConnections() {
        QGroundControl.linkManager.disconnectAll()
        var reason = "Connection unavailable during firmware update"
        QGroundControl.linkManager.setConnectionsSuspended(reason)
    }

    function allowVehicleConnections() {
        QGroundControl.linkManager.setConnectionsAllowed()
    }

    FirmwareUpgradeController {
        id: fwUpgradeController
        signal edgeNotInitialized
        signal edgeInitialized
        signal flashingFinished
        signal flashingFailed
        signal autoUpdateEnabled
        signal manualUpdateEnabled

        function flashingCancelledState() {
            var msg = "Firmare upgrading cancelled. " +
                      "If you want to try again, unplug and plug in your device."
            statusTextArea.appendInfoMessage(msg)
        }

        onDeviceInitialized: {
            status ? edgeInitialized() : edgeNotInitialized()
        }

        onDeviceFlashed: {
            status ? flashingFinished() : flashingFailed()
        }

        onUpdateMethodChanged: {
            updateMethod === FirmwareUpgradeController.Auto ?
                autoUpdateEnabled() :
                manualUpdateEnabled()
        }

        onFlasherProgressChanged: {
            progressBar.changeProgressBarValue(progress);
        }

        onInfoMsgReceived:  { statusTextArea.appendInfoMessage(message); }
        onErrorMsgReceived: { statusTextArea.appendErrorMessage(message); }
        onWarnMsgReceived:  { statusTextArea.appendWarnMessage(message); }

        Component.onDestruction: {
            allowVehicleConnections()
        }
    }

    DSM.StateMachine {
        id:           stateMachine
        initialState: initial
        running:      true

        DSM.State {
           id: initial

           DSM.SignalTransition {
               targetState: devicePlugged
               signal:      fwUpgradeController.devicePlugged
           }

           onEntered: {
               disabledVisible(connectButton)
               notVisible(cancelButton)
               enableVehicleSetupButtons()

               progressBar.setVisibleProgressLabel(false)
               progressBar.changeProgressBarValue(0)
               fwUpgradeController.observeDevice()
               allowVehicleConnections()
           }
        }

        DSM.State {
            id: devicePlugged

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.deviceUnplugged
                onTriggered: {
                    var msg = "Device unplugged."
                    statusTextArea.appendInfoMessage(msg)
                }
            }

            DSM.SignalTransition {
                targetState: deviceInitializationStarted
                signal:      fwUpgradeController.deviceInitializationStarted
            }

            onEntered: {
                enabledVisible(connectButton)
                notVisible(cancelButton)

                var msg = "Emlid edge device found. Press \"Connect\" to continue."
                statusTextArea.appendInfoMessage(msg)
            }
        }


        DSM.State {
            id: deviceInitializationStarted

            DSM.SignalTransition {
                targetState: deviceInitialized
                signal:      fwUpgradeController.edgeInitialized
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.edgeNotInitialized
                onTriggered: {
                    var msg = "An error ocurred while initializing." +
                          "If you want to try again - unplug and plug in your device."
                    statusTextArea.appendErrorMessage(msg)
                }
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.cancelled
                onTriggered: {
                    fwUpgradeController.flashingCancelledState()
                }
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.connectionWithUpdaterAborted
                onTriggered: {
                    fwUpgradeController.flashingCancelledState()
                }
            }

            onEntered: {
                disabledVisible(connectButton)
                notVisible(cancelButton)
                disableVehicleSetup()
                statusTextArea.clear()
            }
        }

        DSM.State {
            id: deviceInitialized

            DSM.SignalTransition {
                targetState: flashingStarted
                signal:      fwUpgradeController.firmwareUpgraderStarted
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.cancelled
                onTriggered: { fwUpgradeController.flashingCancelledState() }
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.connectionWithUpdaterAborted
                onTriggered: {
                    fwUpgradeController.flashingCancelledState()
                }
            }

            onEntered: {
                var dialogTitle = "Firmware settings"

                showDialog(firmwareSettingsComponent,
                       dialogTitle, qgcView.showDialogDefaultWidth,
                       StandardButton.Ok | StandardButton.Cancel)
                disallowVehicleConnections()
            }
        }

        DSM.State {
            id: flashingStarted

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.flashingFinished
                onTriggered: {
                    var msg = "Firmware upgrading finished. Please unplug your device."
                    statusTextArea.appendInfoMessage(msg)
                }
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.flashingFailed
                onTriggered: {
                    var msg = "An error ocurred while flashing device." +
                          "Make sure that you did not disconnect your device."
                    statusTextArea.appendErrorMessage(msg)
                    initialState()
                }
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.cancelled
                onTriggered: {
                    fwUpgradeController.flashingCancelledState()
                }
            }

            DSM.SignalTransition {
                targetState: initial
                signal:      fwUpgradeController.connectionWithUpdaterAborted
                onTriggered: {
                    fwUpgradeController.flashingCancelledState()
                }
            }

            onEntered: {
                notVisible(connectButton)
                enabledVisible(cancelButton)
                progressBar.setVisibleProgressLabel(true)
            }
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
            value:               0
            maximumValue:        100
            width:               parent.width - Math.max(cancelButton.width, connectButton.width) - 2 * ScreenTools.defaultFontPixelWidth

            function changeProgressBarValue(value) {
                progressBar.value = value
            }

            function setVisibleProgressLabel(value) {
                progressLabel.visible = value
            }

            Label {
                id: progressLabel

                visible:                  false
                color:                    qgcPal.window
                text:                     progressBar.value + "%"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter:   parent.verticalCenter
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
                fwUpgradeController.initializeDevice()
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
                var dialogTitle = "Upgrading cancellation"
                showDialog(cancellationDialogComponent,
                           dialogTitle, qgcView.showDialogDefaultWidth,
                           StandardButton.Yes | StandardButton.No)
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

            function clear() {
                statusTextArea.remove(0, statusTextArea.length);
            }

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
        id: cancellationDialogComponent

        QGCViewDialog {
            id: cancellationDialog
            width: questionLabel.width + ScreenTools.defaultFontPixelHeight

            function accept() {
                hideDialog();
                fwUpgradeController.cancel()
            }

            QGCLabel {
                id: questionLabel
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Do you really want to cancel upgrading?";
            }
        }

    }

    Component {
        id: firmwareSettingsComponent

        QGCViewDialog {
            id: firmwareSettings
            anchors.fill: parent

            DSM.StateMachine {
                id:           firmwareSettingsStateMachine
                initialState: autoUpdate
                running:      true

                DSM.State {
                    id: autoUpdate

                    DSM.SignalTransition {
                        targetState: manualUpdate
                        signal:      fwUpgradeController.manualUpdateEnabled
                    }

                    onEntered: {
                        availableDiskSpaceLabel.visible = true
                        saveFirmwareLabel.visible = true
                        saveFirmwareItem.visible = true
                        saveFirmwareInfoLabel.visible = true
                        updateMethodInfoLabel.text = "QGC automatically downloads latest "
                                                   + "firmware and updates Edge. You can "
                                                   + "manually select download location. "
                                                   + "By default QGC uses OS temporary "
                                                   + "or Downloads directory."
                        selectLabel.text = "Select folder"
                        browseDirButton.visible = true
                    }

                    onExited: {
                        saveFirmwareInfoLabel.visible     = false
                        availableDiskSpaceLabel.visible = false
                        saveFirmwareLabel.visible       = false
                        saveFirmwareItem.visible        = false
                        browseDirButton.visible         = false
                    }
                }

                DSM.State {
                    id: manualUpdate
                    DSM.SignalTransition {
                        targetState: autoUpdate
                        signal:      fwUpgradeController.autoUpdateEnabled
                    }

                    onEntered: {
                        updateMethodInfoLabel.text = "You can manually select firmware.";
                        selectLabel.text = "Select image"
                        browseFileButton.visible = true
                    }

                    onExited: {
                        browseFileButton.visible = false
                    }
                }

                Component.onCompleted: {
                    cbUpdateMethod.setAuto()
                }
            }

            function accept() {
                if (fwUpgradeController.updateMethod == FirmwareUpgradeController.Manual
                &&  fwUpgradeController.firmwareFilename == "") {
                    statusTextArea.appendWarnMessage("Firmware file not selected.")
                } else if (fwUpgradeController.updateMethod == FirmwareUpgradeController.Auto
                           && !fwUpgradeController.hasEnoughDiskSpace()){
                    statusTextArea.appendWarnMessage("This location doesn't have enough disk space. You can change this in 'Advanced settings'.")
                } else {
                    hideDialog()
                    fwUpgradeController.flash()
                }
            }

            function reject() {
                hideDialog();
                fwUpgradeController.cancelled()
            }

            QGCFlickable {
                anchors.fill:  parent
                contentHeight: contentColumn.height

            Column {
                id:              contentColumn
                anchors          { right: parent.right; left: parent.left }
                spacing:         ScreenTools.defaultFontPixelHeight
                anchors.margins: ScreenTools.defaultFontPixelWidth * 2

                readonly property int innerElementWidth: width - ScreenTools.defaultFontPixelWidth * 2

                QGCLabel {
                    id:                  firmwareVersionInfoLabel
                    width:               parent.width
                    wrapMode:            Text.WordWrap
                    anchors              { left: parent.left; right: parent.right }
                    font.pointSize:      ScreenTools.defaultFontPointSize - 1
                    horizontalAlignment: Text.AlignHCenter

                    Connections {
                        target:            fwUpgradeController
                        onFirmwareInfoMsg: firmwareVersionInfoLabel.text = message
                    }
                }

                QGCLabel {
                    id:                  availableDiskSpaceLabel
                    width:               parent.width
                    text:                "Available disk space: " + fwUpgradeController.availableDiskSpace
                    wrapMode:            Text.WordWrap
                    anchors              { left: parent.left; right: parent.right }
                    font.pointSize:      ScreenTools.defaultFontPointSize - 1
                    horizontalAlignment: Text.AlignHCenter
                    visible:             false
                }

                QGCDropDownSetting {
                    width:        parent.width
                    title:        "Firmware Info"
                    initialState: openedState

                    onStateChanged: {
                        firmwareInfoColumn.visible = currentState == openedState
                    }
                }


                Rectangle {
                    id:     firmwareInfoColumn
                    color:  qgcPal.windowShade
                    width:  parent.width
                    height: info.height + ScreenTools.defaultFontPixelWidth * 2

                    GridLayout {
                        id:         info
                        rowSpacing: ScreenTools.defaultFontPixelWidth
                        columns:    2
                        rows:       2
                        anchors {
                            margins:          ScreenTools.defaultFontPixelWidth
                            verticalCenter:   parent.verticalCenter
                            left:             parent.left
                            right:            parent.right
                        }

                        QGCLabel {  text: "Needed disk space" }
                        QGCLabel {
                            anchors.right: parent.right
                            id:            downloadSize
                            text:          fwUpgradeController.remoteFirmwareInfo.diskSpace
                        }

                        QGCLabel { text: "Release date" }
                        QGCLabel {
                            anchors.right: parent.right
                            id:            releaseDate
                            text:          fwUpgradeController.remoteFirmwareInfo.releaseDate
                        }

                        QGCLabel { text: "Available version" }
                        QGCLabel {
                            anchors.right: parent.right
                            id:            availableFirmwareVersion
                            text:          fwUpgradeController.remoteFirmwareInfo.version
                        }

                        QGCLabel { text: "Current version" }
                        QGCLabel {
                            anchors.right: parent.right
                            id:            firmwareVersion
                            text:          fwUpgradeController.firmwareVersion
                        }
                    }
                }

                QGCDropDownSetting {
                    width: parent.width
                    title: "Advanced settings"

                    onStateChanged: {
                        advancedSettings.visible = currentState === openedState
                    }
                }

                Rectangle {
                    id:      advancedSettings
                    color:   qgcPal.windowShade
                    width:   parent.width
                    height:  settings.height + ScreenTools.defaultFontPixelWidth * 2
                    visible: false


                    GridLayout {
                        id:                   settings
                        rowSpacing:           ScreenTools.defaultFontPixelWidth
                        width:                parent.width
                        columns:              2
                        rows:                 3
                        anchors {
                            margins:          ScreenTools.defaultFontPixelWidth
                            verticalCenter:   parent.verticalCenter
                            left:             parent.left
                            right:            parent.right
                        }

                        property int __buttonHeight:        ScreenTools.defaultFontPixelWidth * 3
                        property int __buttonWidth:         ScreenTools.defaultFontPixelWidth * 10
                        property int __infoTextFontSize:    ScreenTools.defaultFontPointSize - 2
                        property int __infoTextRightMargin: ScreenTools.defaultFontPixelWidth
                        property int __infoTextLeftMargin:  ScreenTools.defaultFontPixelWidth * 3

                        QGCLabel { text: "Compute checksum" }

                        Item {
                            width:         parent.__buttonWidth
                            height:        parent.__buttonHeight
                            anchors.right: parent.right

                            QGCComboBox {
                                model:        ["Enable", "Disable"]
                                anchors.fill: parent

                                onActivated: {
                                    fwUpgradeController.checksumEnabled =
                                            !fwUpgradeController.checksumEnabled
                                }

                                Component.onCompleted: {
                                    currentIndex = fwUpgradeController.checksumEnabled ? 0 : 1;
                                }
                            }
                        }

                        QGCLabel { text: "Update method" }

                        Item {
                            width:         parent.__buttonWidth
                            height:        parent.__buttonHeight
                            anchors.right: parent.right

                            QGCComboBox {
                                id:           cbUpdateMethod
                                model:        ["Auto", "Manual"]
                                anchors.fill: parent

                                function setAuto()   {
                                    currentIndex = 0
                                    fwUpgradeController.updateMethod = FirmwareUpgradeController.Auto
                                }
                                function setManual() {
                                    currentIndex = 1
                                    fwUpgradeController.updateMethod = FirmwareUpgradeController.Manual
                                }

                                onActivated: {
                                    fwUpgradeController.updateMethod = index == 0 ?
                                                FirmwareUpgradeController.Auto :
                                                FirmwareUpgradeController.Manual
                                }
                            }
                        }

                        QGCLabel {
                            text:    "Save firmware"
                            id:      saveFirmwareLabel
                            visible: false
                        }

                        Item {
                            id:            saveFirmwareItem
                            width:         parent.__buttonWidth
                            height:        parent.__buttonHeight
                            anchors.right: parent.right
                            visible:       false

                            QGCComboBox {
                                id:           cbSaveFirmare
                                model:        ["Enable", "Disable"]
                                anchors.fill: parent

                                onActivated: {
                                    fwUpgradeController.firmwareSavingEnabled =
                                            !fwUpgradeController.firmwareSavingEnabled
                                }

                                Component.onCompleted: {
                                    currentIndex = fwUpgradeController.firmwareSavingEnabled ? 0 : 1;
                                }
                            }
                        }

                        QGCLabel {
                            Layout.leftMargin:   parent.__infoTextRightMargin
                            Layout.rightMargin:  parent.__infoTextLeftMargin
                            Layout.fillWidth:    true
                            Layout.columnSpan:   2
                            id:                  saveFirmwareInfoLabel
                            text:                "After updating finishes QGC automatically "
                                               + "remove downloaded files. You can disable "
                                               + "this function."
                            visible:             false
                            wrapMode:            Text.WordWrap
                            font.pointSize:      parent.__infoTextFontSize
                        }

                        QGCLabel {
                            id: selectLabel
                        }

                        Item {
                            width:         ScreenTools.defaultFontPixelWidth * 10
                            height:        ScreenTools.defaultFontPixelWidth * 3
                            anchors.right: parent.right

                            QGCButton {
                                id:           browseFileButton
                                anchors.fill: parent
                                text:         "Browse"
                                visible:      false

                                onClicked: { fwUpgradeController.askForFirmwareFile() }
                            }

                            QGCButton {
                                id:           browseDirButton
                                anchors.fill: parent
                                text:         "Browse"
                                visible:      false

                                onClicked: { fwUpgradeController.askForFirmwareDirectory() }
                            }
                        }

                        QGCLabel {
                            Layout.leftMargin: parent.__infoTextRightMargin
                            Layout.rightMargin:  parent.__infoTextLeftMargin
                            Layout.fillWidth:  true
                            Layout.columnSpan: 2
                            id:                updateMethodInfoLabel
                            wrapMode:          Text.WordWrap
                            font.pointSize:    parent.__infoTextFontSize
                        }
                    }
                }

                move: Transition {
                    NumberAnimation { properties: "x,y"; duration: 200 }
                }
            }
        }
        }
    }
} // QGCView
