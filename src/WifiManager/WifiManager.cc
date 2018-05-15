#include "WifiManager.h"
#include "QGCMAVLink.h"
#include "Vehicle.h"
#include "QGCApplication.h"

using Base = WifiManagerBase;

namespace util {
    constexpr auto showMavCmdError = true;
    constexpr auto maxRetryCount = 1;

    MAVLinkProtocol& mavlinkProtocol(void) {
        return *(qgcApp()->toolbox()->mavlinkProtocol());
    }

    QByteArray toRawString(QString string, int rawSize)
    {
        auto rawBytes = QByteArray(rawSize, '\0');
        string.squeeze();
        return rawBytes.replace(0, string.length(), string.toUtf8());
    }

    QString decodeString(char const* str, int size) {
        return QString::fromUtf8(QByteArray(str, size).append('\0'));
    }
}


WifiManager::WifiManager(Vehicle* vehicle, QObject* parent)
    : Base(parent),
      _savedNetworksCount(-1),
      _receivedSavedNetworksCount(0),
      _retryCount(util::maxRetryCount),
      _vehicle(vehicle)
{
    _netwksListRequestTimer.setSingleShot(true);
    _netwksListRequestTimer.setInterval(2000);

    QObject::connect(&_netwksListRequestTimer, &QTimer::timeout,
                     this, &WifiManager::_onNetworksListRequestTimeout);

    QObject::connect(_vehicle, &Vehicle::mavlinkMessageReceived,
                     this,     &WifiManager::_onMavlinkMessageReceived);

    QObject::connect(_vehicle, &Vehicle::connectionLostChanged,
                     this,     &WifiManager::_handleConnectionLost);

    QObject::connect(this, &WifiManager::_savedNetworksCountReceived,
                     this, &WifiManager::_onSavedNetworksCountReceived);
}


bool WifiManager::_switchToAccessPoint(void)
{
   _vehicle->sendMavCommand(MAV_COMP_ID_WIFI, MAV_CMD_WIFI_START_AP, true, 1);
   return true;
}


bool WifiManager::_switchToClient(QString const& ssid)
{
    mavlink_message_t msg;
    auto rawSsid = util::toRawString(ssid, ssidMaxLength());

    ::mavlink_msg_wifi_network_connect_pack(util::mavlinkProtocol().getSystemId(),
                                            util::mavlinkProtocol().getComponentId(),
                                            &msg, rawSsid.data());

    _vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg);
    _messageQueue.push({MAVLINK_MSG_ID_WIFI_NETWORK_CONNECT});
    return true;
}


bool WifiManager::_addNetwork(QString const& ssid,
                              QString const& passwd,
                              WifiNetworkInfo::SecurityType secType)
{
    mavlink_message_t msg;

    auto rawSsid   = util::toRawString(ssid, ssidMaxLength());
    auto rawPasswd = util::toRawString(passwd, passwordMaxLength());

    mavlink_msg_wifi_network_add_pack(util::mavlinkProtocol().getSystemId(),
                                      util::mavlinkProtocol().getComponentId(),
                                      &msg, rawSsid.data(), rawPasswd.data(),
                                      static_cast<uint16_t>(secType));

    _vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg);
    _messageQueue.push({MAVLINK_MSG_ID_WIFI_NETWORK_ADD, {ssid, secType}});

    return true;
}


bool WifiManager::_deleteNetwork(QString const& ssid)
{

    mavlink_message_t msg;
    auto rawSsid = util::toRawString(ssid, ssidMaxLength());

    ::mavlink_msg_wifi_network_delete_pack(util::mavlinkProtocol().getSystemId(),
                                           util::mavlinkProtocol().getComponentId(),
                                           &msg, rawSsid);

    _vehicle->sendMessageOnLink(_vehicle->priorityLink(), msg);
    _messageQueue.push({MAVLINK_MSG_ID_WIFI_NETWORK_DELETE, {ssid}});

    return true;
}


void WifiManager::_onMavlinkMessageReceived(mavlink_message_t const& msg)
{
    if (msg.compid != MAV_COMP_ID_WIFI || msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
        return;
    }

    switch (msg.msgid) {
        case MAVLINK_MSG_ID_WIFI_ACK:
            _handleWifiAck(msg);
            break;

        case MAVLINK_MSG_ID_WIFI_STATUS:
            _handleWifiStatus(msg);
            break;

        case MAVLINK_MSG_ID_WIFI_NETWORK_INFO:
            _handleWifiNetworkInfo(msg);
            break;

        case MAVLINK_MSG_ID_WIFI_NETWORKS_COUNT:
            _handleWifiNetworksCount(msg);
            break;

        default:
            auto errmsg = QString("Unrecognized message from WiFi component %1")
                              .arg(msg.msgid);
            _setErrorString(errmsg);
    }
}


void WifiManager::_onNetworksListRequestTimeout(void)
{
    if (_retryCount == 0) {
        _netwksListRequestTimer.stop();
        _retryCount = util::maxRetryCount;
        _setErrorString("Problem with connection. "
                        "Can not load networks list.");
    } else {
        _retryCount--;
        _netwksListRequestTimer.start();
        _requestSavedNetworksInfo();
    }
}


void WifiManager::_onSavedNetworksCountReceived(void)
{
    _netwksListRequestTimer.start();
    _requestSavedNetworksInfo();
}


void WifiManager::_requestWifiStatus(void)
{
    _vehicle->sendMavCommand(MAV_COMP_ID_WIFI,
                             MAV_CMD_GET_WIFI_STATUS,
                             util::showMavCmdError);
}


void WifiManager::_requestSavedNetworksCount(void)
{
    _vehicle->sendMavCommand(MAV_COMP_ID_WIFI,
                             MAV_CMD_GET_WIFI_NETWORKS_COUNT,
                             util::showMavCmdError,
                             NetworkType::Saved);
}


void WifiManager::_requestSavedNetworksInfo(void)
{
    _vehicle->sendMavCommand(MAV_COMP_ID_WIFI,
                             MAV_CMD_GET_WIFI_NETWORKS_INFO,
                             util::showMavCmdError,
                             NetworkType::Saved);
}


void WifiManager::_handleWifiAck(mavlink_message_t const& msg)
{
    if (_messageQueue.empty()) {
        _setErrorString("Wifi ack received, but message queue is empty");
        return;
    }

    mavlink_wifi_ack_t wifiAck;
    ::mavlink_msg_wifi_ack_decode(&msg, &wifiAck);

    if (wifiAck.message_id != _messageQueue.front().msgid) {
        auto msg = QString("Invalid sequence. Received ack on command, "
                           "which is not latest. Latest msg is %1. "
                           "Msg queue size %2").arg(_messageQueue.front().msgid).arg(_messageQueue.size());

        _setErrorString(msg);
        return;
    }

    if (wifiAck.result != 0) {
        auto message = QString("Command %1 is not performed.")
                              .arg(wifiAck.message_id);
        _setErrorString(std::move(message));
    }

    switch (wifiAck.message_id) {
        case MAVLINK_MSG_ID_WIFI_NETWORK_DELETE: {
            auto netwkSsid = _messageQueue.front().args.first().toString();
            Base::_removeNetworkFromList(netwkSsid);
            break;
        }

        case MAVLINK_MSG_ID_WIFI_NETWORK_ADD: {
            auto const& msg = _messageQueue.front();
            auto netwkSsid    = msg.args.value(0).toString();
            auto netwkSecType = msg.args.value(1)
                    .value<WifiNetworkInfo::SecurityType>();

            Base::_addNetworkToList(
                WifiNetworkInfo{std::move(netwkSsid), netwkSecType}
            );

            break;
        }
    }

    _messageQueue.pop();
}


void WifiManager::_handleWifiStatus(mavlink_message_t const& msg)
{
    mavlink_wifi_status_t wifiStatus;
    ::mavlink_msg_wifi_status_decode(&msg, &wifiStatus);

    auto ssid = util::decodeString(wifiStatus.ssid,
                                   sizeof(wifiStatus.ssid));

    Base::_setWifiState(static_cast<WifiState>(wifiStatus.state));
    Base::_setActiveNetworkSsid(std::move(ssid));
}


void WifiManager::_handleWifiNetworkInfo(mavlink_message_t const& msg)
{
    mavlink_wifi_network_info_t netwkInfo;
    ::mavlink_msg_wifi_network_info_decode(&msg, &netwkInfo);

    if (_savedNetworksCount == -1) {
        _setErrorString("Networks info is received before networks count");

    } else if (netwkInfo.type == NetworkType::Saved) {
        auto ssid = util::decodeString(netwkInfo.ssid, sizeof(netwkInfo.ssid));

        _receivedSavedNetworksCount++;

        if (_receivedSavedNetworksCount == _savedNetworksCount) {
            _netwksListRequestTimer.stop();
            _receivedSavedNetworksCount = 0;

        } else {
            _netwksListRequestTimer.start();
        }

        if (Base::_listContainsNetwork(ssid)) {
            return;
        }

        using SecType = WifiNetworkInfo::SecurityType;
        Base::_addNetworkToList(
            WifiNetworkInfo{
                std::move(ssid),
                static_cast<SecType>(netwkInfo.security_type)
            }
        );
    }
}


void WifiManager::_handleWifiNetworksCount(mavlink_message_t const& msg)
{
    mavlink_wifi_networks_count_t netwkCount;
    ::mavlink_msg_wifi_networks_count_decode(&msg, &netwkCount);

    if (netwkCount.type == NetworkType::Saved) {
        _savedNetworksCount = netwkCount.count;
        emit _savedNetworksCountReceived();
    }
}


void WifiManager::_handleConnectionLost(bool isConnectionLost)
{
    if (isConnectionLost) {
        if (Base::wifiState() != WifiState::Switching) {
            Base::_setWifiState(WifiState::Undefined);
        }
        return;
    }

    _requestWifiStatus();
    _requestSavedNetworksCount();
}
