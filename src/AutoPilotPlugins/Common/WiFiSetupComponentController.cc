/****************************************************************************
 *
 *   (c) 2009-2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#include "WiFiSetupComponentController.h"
#include "QGCMAVLink.h"
#include "QGCApplication.h"

WiFiSetupComponentController::WiFiSetupComponentController()
    : _connectionName("")
    , _connectionType("")
{
    _protocolTypes.append("OPEN");
    _protocolTypes.append("WEP");
    _protocolTypes.append("WPA");
    _protocolTypes.append("WPA2");

    connect(_vehicle, &Vehicle::mavlinkWifiNetworkInformation, this, &WiFiSetupComponentController::_handleWiFiNetworkInformation);

    update_network_list();
}

WiFiSetupComponentController::~WiFiSetupComponentController()
{

}

void WiFiSetupComponentController::update_network_list()
{
    _networks.clear();

    // Request for list of networks
    _vehicle->sendMavCommand(MAV_COMP_ID_WIFI, MAV_CMD_REQUEST_WIFI_NETWORKS, true, 1);
}

void WiFiSetupComponentController::startAPMode()
{
    _vehicle->sendMavCommand(MAV_COMP_ID_WIFI, MAV_CMD_WIFI_START_AP, true, 1);
}

void WiFiSetupComponentController::connectToNetwork(const QString name)
{
    mavlink_message_t msg;

    mavlink_msg_wifi_network_connect_pack(qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                                          qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                                          &msg,
                                          name.toStdString().c_str());

    _vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg);
}

void WiFiSetupComponentController::addNetwork(const QString name, const int type, const QString psw)
{
    mavlink_message_t msg;

    mavlink_msg_wifi_network_add_pack(qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                                      qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                                      &msg,
                                      name.toStdString().c_str(),
                                      type,
                                      psw.toStdString().c_str());

    _vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg);
    update_network_list();
}

void WiFiSetupComponentController::removeNetwork(const QString name)
{
    mavlink_message_t msg;

    mavlink_msg_wifi_network_delete_pack(qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                                         qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                                         &msg,
                                         name.toStdString().c_str());

    _vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg);
    update_network_list();
}

void WiFiSetupComponentController::_handleWiFiNetworkInformation(mavlink_message_t message)
{
    mavlink_wifi_network_information_t wifiNetworkInformation;
    mavlink_msg_wifi_network_information_decode(&message, &wifiNetworkInformation);

    QString name = QString(wifiNetworkInformation.ssid);

    _networks << name;
    emit _networksChanged();

    if (wifiNetworkInformation.state) {
        _connectionName = name;
        _connectionType = _protocolTypes[wifiNetworkInformation.security_type];
    }
}
