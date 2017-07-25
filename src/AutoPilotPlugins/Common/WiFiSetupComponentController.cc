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

    // Request for list of networks
    _vehicle->sendMavCommand(MAV_COMP_ID_WIFI, MAV_CMD_REQUEST_WIFI_NETWORKS, true, 1);
}

WiFiSetupComponentController::~WiFiSetupComponentController()
{

}

void WiFiSetupComponentController::startAPMode()
{
    _vehicle->sendMavCommand(MAV_COMP_ID_WIFI, MAV_CMD_WIFI_START_AP, true, 1);
}

void WiFiSetupComponentController::connectToNetwork(const QString name)
{
    mavlink_message_t msg;

    const char *mav_name;

    mav_name = name.toStdString().c_str();

    mavlink_msg_wifi_network_connect_pack_chan(qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                                               qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                                               _vehicle->priorityLink()->mavlinkChannel(),
                                               &msg,
                                               mav_name);
}

void WiFiSetupComponentController::addNetwork(const QString name, const int type, const QString psw)
{
    mavlink_message_t msg;

    const char *mav_name, *mav_psw;

    mav_name = name.toStdString().c_str();
    mav_psw = psw.toStdString().c_str();

    mavlink_msg_wifi_network_add_pack_chan(qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                                           qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                                           _vehicle->priorityLink()->mavlinkChannel(),
                                           &msg,
                                           mav_name,
                                           type,
                                           mav_psw);
}

void WiFiSetupComponentController::removeNetwork(const QString name)
{
    mavlink_message_t msg;

    const char *mav_name;

    mav_name = name.toStdString().c_str();

    mavlink_msg_wifi_network_delete_pack_chan(qgcApp()->toolbox()->mavlinkProtocol()->getSystemId(),
                                              qgcApp()->toolbox()->mavlinkProtocol()->getComponentId(),
                                              _vehicle->priorityLink()->mavlinkChannel(),
                                              &msg,
                                              mav_name);
}

void WiFiSetupComponentController::_handleWiFiNetworkInformation(mavlink_message_t message)
{
    mavlink_wifi_network_information_t wifiNetworkInformation;
    mavlink_msg_wifi_network_information_decode(&message, &wifiNetworkInformation);

    QString name = QString(wifiNetworkInformation.ssid);

    _networks << name;

    if (wifiNetworkInformation.state) {
        _connectionName = name;
        _connectionType = _protocolTypes[wifiNetworkInformation.security_type];
    }
}
