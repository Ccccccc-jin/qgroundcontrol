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

            property real   _margins:      ScreenTools.defaultFontPixelHeight / 2
            property int    _firstColumn:  ScreenTools.defaultFontPixelWidth * 15
            property int    _secondColumn: ScreenTools.defaultFontPixelWidth * 25
            property string _currentNetwork: ""

            Column {
                width:    ScreenTools.defaultFontPixelWidth * 55
                spacing: _margins

                QGCLabel { text: qsTr("Status") }

                Rectangle {
                    anchors { left: parent.left; right: parent.right }
                    height: 50
                    color:  palette.windowShade

                    GridLayout {
                        anchors  { fill: parent; margins: _margins }
                        columns: 2

                        QGCLabel { id: modeLabel }

                        Switch {
                            id:            modeSwitch
                            anchors.right: parent.right

                            function switchToAccessPointMode() {
                                modeLabel.text = "Access point mode"
                                controller.bootAsAccessPoint()
                            }

                            function switchToClientMode() {
                                modeLabel.text = "Client mode"
                                controller.bootAsClient(controller.defaultNetwork)
                            }

                            onClicked: {
                                if (checked) {
                                    switchToAccessPointMode()
                                } else {
                                    switchToClientMode()
                                }
                            }

                            style: SwitchStyle {
                                id: switchStyle

                                readonly property string flatBlueColor:  "#3498db"
                                readonly property string flatGreenColor: "#2ecc71"

                                groove: Rectangle {
                                    implicitWidth:  50
                                    implicitHeight: modeLabel.height
                                    radius:         2
                                    color:          control.checked ?
                                                        switchStyle.flatBlueColor : switchStyle.flatGreenColor
                                    border.width:   1
                                    border.color:   "grey"
                                }
                            }
                        }

                        Component.onCompleted:  {
                            if (controller.edgeMode === controller.AccessPoint) {
                                modeLabel.text = "Access point mode"
                                modeSwitch.checked = true
                            } else {
                                modeLabel.text = "Client mode"
                                modeSwitch.checked = false
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
                    height: width * 1.2
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

                            function currentListElement() {
                                return model[currentIndex]
                            }

                            function hasSelectedElement() {
                                return currentIndex !== -1
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

                            model: controller.savedNetworks

                            delegate: Item {
                                height: ScreenTools.defaultFontPixelHeight * 3
                                width:  savedNetworksListView.width

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: _margins / 2

                                    QGCLabel {
                                        id:             networkLabel
                                        text:           modelData
                                        font.pointSize: ScreenTools.mediumFontPointSize
                                    }

                                    Row {
                                        id: networkStatusRow
                                        spacing: _margins / 2

                                        readonly property real
                                            statusLabelFontSize: ScreenTools.smallFontPointSize * 1.2

                                        QGCLabel {
                                            text:           "Default"
                                            visible:        modelData === controller.defaultNetwork
                                            font.pointSize: parent.statusLabelFontSize
                                            color:          palette.primaryButton
                                        }

                                        QGCLabel {
                                            text:           "Connected"
                                            visible:        modelData === controller.activeNetwork
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

                            Component.onCompleted: {
                                currentIndex = -1
                            }
                        }

                        GridLayout {
                            id:      btnsPanel
                            anchors  { left: parent.left; right: parent.right }
                            columns: 4

                            property int _btnsHeight: ScreenTools.defaultFontPixelHeight * 3

                            Item {
                                Layout.fillWidth: true
                                height:  parent._btnsHeight
                                enabled: savedNetworksListView.hasSelectedElement()

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
                                    title:           qsTr("Connect to Network: %1")
                                        .arg(savedNetworksListView.currentListElement())
                                    text:            qsTr("Do you want to connect to a selected Wi-Fi network?")

                                    onYes: {
                                        var currentNetwkName = savedNetworksListView
                                            .currentListElement()

                                        controller.bootAsClient(currentNetwkName)
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
                                enabled: savedNetworksListView.hasSelectedElement()

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
                                    title:           qsTr("Connect to Network: %1")
                                        .arg(savedNetworksListView.currentListElement())
                                    text:            qsTr("Do you want to remove selected Wi-Fi network from list?")

                                    onYes: {
                                        var currentNetwkName = savedNetworksListView
                                            .currentListElement()

                                        controller.removeNetworkFromEdge(currentNetwkName)
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
                                enabled: savedNetworksListView.hasSelectedElement()

                                QGCButton {
                                    anchors.fill: parent
                                    text: "Set as default"
                                    onClicked: {
                                        var netwkIdx = savedNetworksListView.currentIndex
                                        var netwkName = controller.networks[netwkIdx]
                                        controller.defaultNetwork = netwkName
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

                    function isPasswdValid(passwd) {
                        return (passwd === "" || passwd.length < 8)
                    }

                    function areTheFieldsCorrect(networkSsid, networkEncryptionType, networkPasswd) {
                        if (networkSsid === "") {
                            warningPanel.show("Network wasn't selected")

                        } else if (networkEncryptionType !== WiFiSetupComponentController.OpenEncrypt
                                   && isPasswdValid(networkPasswd)) {
                            warningPanel.show("Password should contains at least 8 characters")

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
                            networkEncryptionType = controller
                                .encryptTypeStrings.indexOf(scannedNetworksList.selectedNetworkEncryptionType)
                        } else {
                            networkSsid = networkNameField.text
                            networkEncryptionType = encryptionTypeCombo.currentIndex
                        }

                        if (areTheFieldsCorrect(networkSsid, networkEncryptionType, networkPasswd)) {
                            controller.saveNetworkToEdge(networkSsid,
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

                                                    onClicked: {
                                                        hiddenNetworkButton.checked = false
                                                        networkListPanel.show()

                                                        networkNamePanel.hide()
                                                        encryptionTypePanel.hide()
                                                        passwordPanel.hide()
                                                        warningPanel.hide()
                                                    }
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

                                                    onClicked: {
                                                        networkListButton.checked = false
                                                        networkListPanel.hide()

                                                        networkNamePanel.show()
                                                        encryptionTypePanel.show()
                                                        passwordPanel.hide()
                                                        warningPanel.hide()
                                                    }
                                                }
                                            }

                                            Component.onCompleted: {
                                                networkListButton.checked = true
                                            }
                                        }
                                    }

                                    Column {
                                        id:      networkListPanel
                                        width:   parent.width
                                        spacing: parent.childSpacing()

                                        function hide() { visible = false; scannedNetworksList.currentIndex = -1 }
                                        function show() { visible = true }

                                        QGCLabel { text: qsTr("Choose network from list") }

                                        QGCListView {
                                            id:      scannedNetworksList
                                            anchors  { left: parent.left; right: parent.right }
                                            height:  ScreenTools.defaultFontPixelHeight * 10
                                            focus:   true
                                            clip:    true

                                            property string _mainColor: "grey"
                                            property string selectedNetworkSsid: ""
                                            property string selectedNetworkEncryptionType: ""

                                            keyNavigationEnabled: true
                                            headerPositioning: ListView.OverlayFooter
                                            footerPositioning: headerPositioning

                                            header: Rectangle {
                                                width:  scannedNetworksList.width;
                                                color:  scannedNetworksList._mainColor
                                                height: 1
                                            }

                                            footer: header

                                            model: controller.scannedNetworks

                                            delegate: Item {
                                                height: networkInfoColumn.height; width: scannedNetworksList.width

                                                Column {
                                                    id:     networkInfoColumn
                                                    anchors {
                                                        left: parent.left
                                                        right: parent.right
                                                        leftMargin: _margins
                                                    }

                                                    QGCLabel { text: ssid }

                                                    QGCLabel {
                                                        text:  encryptionType
                                                        color: palette.primaryButton
                                                    }
                                                }

                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked:    {
                                                        scannedNetworksList.currentIndex = index
                                                        var openEncryptionType = controller
                                                            .encryptionTypeAsString(controller.OpenEncrypt)

                                                        if (encryptionType === openEncryptionType) {
                                                            passwordPanel.hide()
                                                        } else {
                                                            passwordPanel.show()
                                                        }

                                                        scannedNetworksList.selectedNetworkSsid = ssid
                                                        scannedNetworksList.selectedNetworkEncryptionType = encryptionType
                                                    }
                                                }
                                            }

                                            highlight: Rectangle { color: scannedNetworksList._mainColor }

                                            Component.onCompleted: {
                                                currentIndex = -1
                                            }
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
                                            maximumLength:  controller.ssidMaxLength
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
                                            model:   controller.encryptTypeStrings

                                            onActivated: {
                                                console.log(index)
                                                console.log(WiFiSetupComponentController.OpenEncrypt)
                                                if (index === WiFiSetupComponentController.OpenEncrypt) {
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
                                            maximumLength:  controller.passwdMaxLength
                                            echoMode:       TextInput.Password
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
