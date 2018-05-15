/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/



import QtQuick                 2.7
import QtQuick.Controls        1.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs         1.2
import QtQuick.Layouts         1.2

import QGroundControl               1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.ScreenTools   1.0


SetupPage {
    id:             wifiPage
    pageComponent:  pageComponent

    property WifiManager _wifiManager: QGroundControl.multiVehicleManager
                                             .activeVehicle
                                             .wifiManager
    function wifiStateAsString(wifiState) {
        switch(wifiState) {
            case WifiManager.AccessPoint: return "Access Point"
            case WifiManager.Client:      return "Client mode"
            case WifiManager.Undefined:   return "Undefined"
            case WifiManager.Switching:   return "Switching..."
        }
    }

    property var corePlugin: QGroundControl.corePlugin

    function enableVehicleSetupButtons()  { corePlugin.vehicleSetupDisabled = false }
    function disableVehicleSetupButtons() { corePlugin.vehicleSetupDisabled = true }

    Component {
        id: pageComponent

        Column {
            id:       settingColumn
            spacing:  _margins
            width:    availableWidth

            function setComponentActiveState() {
                btnsPanel.enablePanel()
                savedNetworksListView.enableView()
                savedNetworksListView.resetCurrentIndex()
                enableVehicleSetupButtons()
            }

            function setComponentInactiveState() {
                btnsPanel.disablePanel()
                savedNetworksListView.disableView()
            }

            function updateWifiState(wifiState) {
                switch(wifiState) {
                    case WifiManager.AccessPoint:
                        modeSwitch.setAccessPointMode()
                        setComponentActiveState()
                        break

                    case WifiManager.Client:
                        modeSwitch.setClientMode()
                        setComponentActiveState()
                        break

                    case WifiManager.Undefined:
                        modeSwitch.setUndefined()
                        setComponentInactiveState()
                        break

                    case WifiManager.Switching:
                        modeSwitch.setSwitching()
                        setComponentInactiveState()
                        disableVehicleSetupButtons()
                        break

                    default:
                        console.log("Undefined wifistatus")
                        break
                }
            }

            Connections {
                target: _wifiManager
                onWifiStateChanged: {
                    settingColumn.updateWifiState(_wifiManager.wifiState)
                }

                Component.onCompleted: {
                    modeSwitch.setUndefined()
                    btnsPanel.disablePanel()
                    savedNetworksListView.disableView()

                    settingColumn.updateWifiState(_wifiManager.wifiState)
                }

                Component.onDestruction: {
                    enableVehicleSetupButtons()
                }
            }

            QGCPalette { id: palette; colorGroupEnabled: true }

            property real   _margins:      ScreenTools.defaultFontPixelHeight / 2
            property int    _firstColumn:  ScreenTools.defaultFontPixelWidth * 15
            property int    _secondColumn: ScreenTools.defaultFontPixelWidth * 25

            Column {
                width:    ScreenTools.defaultFontPixelWidth * 60
                spacing: _margins

                QGCLabel { text: qsTr("Status") }

                Rectangle {
                    anchors { left: parent.left; right: parent.right }
                    height: ScreenTools.defaultFontPixelHeight * 3
                    color:  palette.windowShade

                    GridLayout {
                        anchors  { fill: parent; margins: _margins }
                        columns: 2

                        QGCLabel { id: modeLabel }

                        Switch {
                            id:            modeSwitch
                            anchors.right: parent.right

                            readonly property string accessPointColor: "#3498db" // flatBlue
                            readonly property string clientModeColor:  "#2ecc71" // flatGreen

                            function setAccessPointMode() {
                                modeLabel.text = wifiStateAsString(WifiManager.AccessPoint)
                                checked = true
                                enabled = true
                            }

                            function setClientMode() {
                                modeLabel.text = wifiStateAsString(WifiManager.Client)
                                checked = false
                                enabled = true
                            }

                            function setUndefined() {
                                modeLabel.text = wifiStateAsString(WifiManager.Undefined)
                                enabled = false
                            }

                            function setSwitching() {
                                modeLabel.text = wifiStateAsString(WifiManager.Switching)
                                enabled = false
                            }

                            onClicked: {
                                if (checked) {
                                    _wifiManager.switchToAccessPoint()

                                } else if (_wifiManager.defaultNetworkSsid.length > 0
                                        && _wifiManager.savedNetworks.contains(_wifiManager.defaultNetworkSsid) ) {
                                    _wifiManager.switchToClient(_wifiManager.defaultNetworkSsid)

                                } else {
                                    switchWarnDialog.visible = true
                                    checked = true
                                }
                            }

                            style: SwitchStyle {
                                id: switchStyle

                                groove: Rectangle {
                                    implicitWidth:  ScreenTools.defaultFontPixelWidth * 7
                                    implicitHeight: modeLabel.height
                                    radius:         2
                                    color:          control.checked ?
                                                        modeSwitch.accessPointColor : modeSwitch.clientModeColor
                                    border.width:   1
                                    border.color:   "grey"
                                }
                            }


                            MessageDialog {
                                id:              switchWarnDialog
                                visible:         false
                                icon:            StandardIcon.Warning
                                standardButtons: StandardButton.Ok
                                title:           qsTr("Connect to Network")
                                text:            qsTr("Can not connect to default network. Default network is unspecified.")


                                onAccepted: { visible = false }
                            }
                        }
                    }
                }

                Item {
                    id:     blank
                    anchors { left: parent.left; right: parent.right }
                    height: _margins / 2
                }

                QGCLabel { text: qsTr("Saved networks") }

                Rectangle {
                    anchors { left: parent.left; right: parent.right }
                    height: width * 1.1
                    color:  palette.windowShade

                    Column {
                        spacing:         _margins
                        anchors.fill:    parent
                        anchors.margins: spacing

                        QGCListView {
                            id:     savedNetworksListView
                            anchors { left: parent.left; right: parent.right }
                            height: parent.height - btnsPanel.height - parent.spacing
                            focus:  true
                            clip:   true

                            property string _mainColor: "grey"

                            function enableView() { enabled = true }
                            function disableView() { enabled = false }

                            function resetCurrentIndex() { currentIndex = -1 }

                            function currentListElement() {
                                return currentIndex != -1 ? model.at(currentIndex) : null
                            }

                            keyNavigationEnabled: true
                            headerPositioning: ListView.OverlayFooter
                            footerPositioning: headerPositioning

                            header: Rectangle {
                                width:  savedNetworksListView.width;
                                color:  savedNetworksListView._mainColor
                                height: 1
                            }

                            footer: header

                            model: _wifiManager.savedNetworks

                            delegate: Item {
                                height: ScreenTools.defaultFontPixelHeight * 3
                                width:  savedNetworksListView.width

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: _margins / 2

                                    QGCLabel {
                                        id:             networkLabel
                                        text:           ssid
                                        font.pointSize: ScreenTools.mediumFontPointSize
                                    }

                                    Row {
                                        id: networkStatusRow
                                        spacing: _margins / 2

                                        readonly property real
                                            statusLabelFontSize: ScreenTools.smallFontPointSize * 1.2

                                        QGCLabel {
                                            text:           "Default"
                                            visible:        ssid === _wifiManager.defaultNetworkSsid
                                            font.pointSize: parent.statusLabelFontSize
                                            color:          palette.primaryButton
                                        }

                                        QGCLabel {
                                            text:           "Connected"
                                            visible:        ssid === _wifiManager.activeNetworkSsid
                                            font.pointSize: parent.statusLabelFontSize
                                            color:          "green"
                                        }
                                    }
                                }

                                MouseArea {
                                    id:              itemMouseArea
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                    width:           parent.width
                                    height:          parent.height

                                    onClicked: {
                                        savedNetworksListView.currentIndex = index
                                    }
                                }
                            }

                            highlight: Rectangle { color: savedNetworksListView._mainColor }
                        }

                        GridLayout {
                            id:      btnsPanel
                            anchors  { left: parent.left; right: parent.right }
                            columns: 4

                            property int _btnsHeight: ScreenTools.defaultFontPixelHeight * 2.3

                            function disablePanel() { enabled = false }
                            function enablePanel() { enabled = true;}

                            Item {
                                Layout.fillWidth: true
                                height:  parent._btnsHeight
                                enabled: savedNetworksListView.count > 0
                                      && savedNetworksListView.currentItem !== null
                                      && savedNetworksListView.currentListElement() !== _wifiManager.activeNetworkSsid

                                QGCButton {
                                    anchors.fill: parent; text: "Connect"
                                    onClicked: {
                                        connectDialog.visible = true
                                    }
                                }

                                MessageDialog {
                                    id:              connectDialog
                                    visible:         false
                                    icon:            StandardIcon.Warning
                                    standardButtons: StandardButton.Yes | StandardButton.No
                                    title:           qsTr("Connect to Network: %1").arg(savedNetworksListView.currentListElement())
                                    text:            qsTr("Do you want to connect to a selected Wi-Fi network?")

                                    onYes: {
                                        var currentNetwkName = savedNetworksListView.currentListElement()

                                        _wifiManager.switchToClient(currentNetwkName)
                                        connectDialog.visible = false
                                    }

                                    onNo: {
                                        connectDialog.visible = false
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                                height: parent._btnsHeight
                                enabled: savedNetworksListView.count > 0
                                      && savedNetworksListView.currentItem !== null

                                QGCButton {
                                    anchors.fill: parent; text: "Delete"
                                    onClicked: {
                                        deleteDialog.visible = true
                                    }
                                }

                                MessageDialog {
                                    id:              deleteDialog
                                    visible:         false
                                    icon:            StandardIcon.Warning
                                    standardButtons: StandardButton.Yes | StandardButton.No
                                    title:           qsTr("Delete network: %1")
                                        .arg(savedNetworksListView.currentListElement())

                                    function colouredText(color, txt) {
                                        return "<font color=\"" + color + "\">" + txt + "</font>"
                                    }

                                    text: _wifiManager.activeNetworkSsid === savedNetworksListView.currentListElement() ?
                                         colouredText("orange", "Warning")
                                             + ": The network is active. If you remove this network, device would return to "
                                             + colouredText(modeSwitch.accessPointColor,"Access point")
                                             + " mode. Do you want to continue?"
                                         : qsTr("Do you want to remove selected Wi-Fi network from list?")

                                    onYes: {
                                        var currentNetwkName = savedNetworksListView
                                            .currentListElement()

                                        _wifiManager.deleteNetwork(currentNetwkName)
                                        connectDialog.visible = false
                                    }

                                    onNo: {
                                        connectDialog.visible = false
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                                height:  parent._btnsHeight
                                enabled: savedNetworksListView.count > 0
                                      && savedNetworksListView.currentItem !== null

                                QGCButton {
                                    anchors.fill: parent
                                    text: "Set by default"
                                    onClicked: {
                                        var netwkIdx =  savedNetworksListView.currentIndex
                                        var netwkName = _wifiManager.savedNetworks.at(netwkIdx)
                                        _wifiManager.defaultNetworkSsid = netwkName === _wifiManager.defaultNetworkSsid ?
                                                    "" : netwkName
                                    }
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                                height: parent._btnsHeight

                                QGCButton {
                                    anchors.fill: parent
                                    text: "Add"
                                    onClicked: {
                                        var dialogTitle = "Add new network"
                                        showDialog(addNetworkComponent,
                                                   dialogTitle, qgcView.showDialogDefaultWidth,
                                                   StandardButton.Ok | StandardButton.Cancel)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Component {
                id: addNetworkComponent

                QGCViewDialog {
                    id: viewDialog
                    anchors.fill: parent

                    function areTheFieldsCorrect(networkSsid, networkEncryptionType, networkPasswd) {
                        if (networkSsid === "") {
                            warningPanel.show("Network wasn't selected")

                        } else if (networkEncryptionType !== WifiNetworkInfo.Open
                                   && !_wifiManager.validatePassword(networkPasswd)) {
                            warningPanel.show("Invalid password. Password should contains at least 8 characters")

                        } else {
                            return true;
                        }
                        return false;
                    }

                    function accept() {
                        var networkSsid
                        var networkEncryptionType
                        var networkPasswd = passwordField.text

                        if (networkListButton.checked) {
                            networkSsid = scannedNetworksList.selectedNetworkSsid
                            networkEncryptionType = _wifiManager.
                                securiptyTypeStringList.indexOf(scannedNetworksList.selectedNetworkEncryptionType)
                        } else {
                            networkSsid = networkNameField.text
                            networkEncryptionType = encryptionTypeCombo.currentIndex
                        }

                        if (areTheFieldsCorrect(networkSsid, networkEncryptionType, networkPasswd)) {
                            _wifiManager.addNetwork(networkSsid,
                                                    networkEncryptionType,
                                                    networkPasswd)
                            hideDialog()
                        }
                    }

                    function reject() {
                        hideDialog()
                    }

                    QGCFlickable {
                        anchors.fill: parent
                        contentHeight: contentColumn.height

                        Column {
                            id:      contentColumn
                            anchors  { left: parent.left; right: parent.right }
                            spacing: _margins

                            Rectangle {
                                color:  palette.windowShade
                                height: width * 1.5
                                anchors {
                                    left:    parent.left
                                    right:   parent.right
                                    margins: _margins
                                }

                                Column {
                                    anchors  { fill: parent; margins: _margins }
                                    spacing: _margins


                                    function childSpacing() { return spacing / 2 }

                                    Column {
                                        width:   parent.width
                                        spacing: parent.childSpacing()
                                        visible: false

                                        GridLayout {
                                            anchors        { left: parent.left; right: parent.right }
                                            columns:       2
                                            columnSpacing: 0

                                            Item {
                                                Layout.fillWidth: true
                                                height: networkListButton.height

                                                QGCButton {
                                                    id:        networkListButton
                                                    checkable: true
                                                    anchors    { left: parent.left; right: parent.right }
                                                    text:      "Visible"

                                                    function accept() {
                                                        hiddenNetworkButton.checked = false
                                                        networkListPanel.show()

                                                        networkNamePanel.hide()
                                                        encryptionTypePanel.hide()
                                                        passwordPanel.hide()
                                                        warningPanel.hide()
                                                    }

                                                    onClicked: { accept() }
                                                }
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                                height: hiddenNetworkButton.height

                                                QGCButton {
                                                    id:        hiddenNetworkButton
                                                    checkable: true
                                                    anchors    { left: parent.left; right: parent.right }
                                                    text:      "Hidden"

                                                    function accept() {
                                                        networkListButton.checked = false

                                                        networkNamePanel.show()
                                                        encryptionTypePanel.show()
                                                        passwordPanel.hide()
                                                        warningPanel.hide()
                                                    }

                                                    onClicked: { accept() }
                                                }
                                            }
                                        }

                                        Component.onCompleted: {
                                            hiddenNetworkButton.checked = true
                                            hiddenNetworkButton.accept()
                                        }
                                    }

                                    Column {
                                        id:      networkNamePanel
                                        width:   parent.width
                                        spacing: parent.childSpacing()
                                        visible: false

                                        function hide() { visible = false; networkNameField.text = "" }
                                        function show() { visible = true }

                                        QGCLabel { text:  qsTr("Network name") }
                                        QGCTextField {
                                            id:             networkNameField
                                            anchors         { left: parent.left; right: parent.right }
                                            maximumLength:  _wifiManager.ssidMaxLength()
                                            validator:      RegExpValidator {
                                                regExp: /[\0040-\0176]*/
                                            }
                                        }
                                    }

                                    Column {
                                        id:      encryptionTypePanel
                                        width:   parent.width
                                        spacing: parent.childSpacing()
                                        visible: false

                                        function hide() { visible = false; encryptionTypeCombo.currentIndex = 0 }
                                        function show() { visible = true }

                                        QGCLabel { text:   qsTr("Type") }
                                        QGCComboBox {
                                            id:      encryptionTypeCombo
                                            anchors  { left: parent.left; right: parent.right }
                                            width:   _secondColumn
                                            model:   [
                                                _wifiManager.securityTypeAsString(WifiNetworkInfo.Open),
                                                _wifiManager.securityTypeAsString(WifiNetworkInfo.WEP),
                                                _wifiManager.securityTypeAsString(WifiNetworkInfo.WPA),
                                                _wifiManager.securityTypeAsString(WifiNetworkInfo.WPA2)
                                            ]

                                            onActivated: {
                                                if (index === WifiNetworkInfo.Open) {
                                                    passwordPanel.hide()
                                                } else {
                                                    passwordPanel.show()
                                                }
                                            }
                                        }
                                    }

                                    Column {
                                        id:      passwordPanel
                                        width:   parent.width
                                        spacing: parent.childSpacing()
                                        visible: false

                                        function hide() { visible = false; passwordField.text = "" }
                                        function show() { visible = true }

                                        QGCLabel { text: qsTr("Password") }
                                        QGCTextField {
                                            id:             passwordField
                                            anchors         { left: parent.left; right: parent.right }
                                            maximumLength:  _wifiManager.passwordMaxLength()
                                            echoMode:       TextInput.Password
                                            validator:      RegExpValidator {
                                                regExp: /[\0040-\0176]*/
                                            }
                                        }

                                        Row {
                                            QGCCheckBox {
                                                id: showPasswdCheckbox
                                                onClicked: {
                                                    passwordField.echoMode = checked ?
                                                                TextInput.Normal : TextInput.Password
                                                }

                                                Component.onCompleted: { checked = false }
                                            }

                                            QGCLabel {
                                                anchors.verticalCenter: parent.verticalCenter
                                                font.pointSize: passwordField.font.pointSize / 1.3
                                                text: "Show password"
                                            }
                                        }
                                    }

                                    Column {
                                        id:      warningPanel
                                        width:   parent.width
                                        spacing: parent.childSpacing()
                                        visible: false

                                        function show(msg) {
                                            warningLabel.text = msg
                                            warningPanel.visible = true
                                        }

                                        function hide() {
                                            warningPanel.visible = false
                                        }


                                        QGCLabel { text: qsTr("Warning"); color: "orange" }
                                        QGCLabel {
                                            id:       warningLabel
                                            width:    parent.width
                                            wrapMode: Text.WordWrap
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
}
